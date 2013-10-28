import sys
import re
import codecs
import psycopg2

import logging
import gensim
from gensim import corpora
from gensim.models.ldamodel import LdaModel

logging.basicConfig(format='%(asctime)s : %(levelname)s : %(message)s', level=logging.INFO)

class TweetCorpus(object):
    
    def __init__(self, file_name, dictionary):
        self.file_name = file_name
        self.dictionary = dictionary
    
    def __iter__(self):
        for line in open(self.file_name):
            yield self.dictionary.doc2bow(line.split()[5:])

class Tweets(object):
    
    def __init__(self, file_name):
        self.file_name = file_name
    
    def __iter__(self):
        for line in open(self.file_name):
            yield line.split()[5:]
            
def save_tweets_topics(lda, dictionary, con, file_name):
    cur = con.cursor()
        
    bulk = []
    run = 0
    
    o = open(file_name + ".sql", "wr", buffering=8092000)
    for line in open(file_name, "r", buffering=8092000):
        vals = line.split()
        words = vals[5:]
        bow = dictionary.doc2bow(words)
        topics = lda[bow]
        
        for t in topics:

            bulk.append((vals[0], t[0], t[1]))
        
        if len(bulk) >= 100000:
            for t in bulk:
                o.write(str(t[0]) + '\t' + str(t[1]) + '\t' + str(t[2]) + '\n')
            logging.info("Persisting bulk %s of size %s" % (run, len(bulk)))
            bulk = []
            run += 1
    """
            cur.executemany("INSERT INTO tweet_topic (tweet_id, topic, topic_value) VALUES (%s,%s,%s) ", bulk)
            con.commit()
            logging.info("Persisted")
    """
    o.close()
    

def create_infra(file_name):
    tweets = Tweets(file_name)
    dictionary = corpora.Dictionary(tweets)
    stop_ids = [dictionary.token2id[stopword.strip()] for stopword in open("stopwords.txt") if stopword.strip() in dictionary.token2id]
    en_stop_ids = [dictionary.token2id[stopword.strip()] for stopword in open("en_stopwords.txt") if stopword.strip() in dictionary.token2id]
    rare_ids = [tokenid for tokenid, docfreq in dictionary.dfs.iteritems() if docfreq <= 20]
    frequent_ids = [tokenid for tokenid, docfreq in dictionary.dfs.iteritems() if docfreq >= 2000000]
    
    dictionary.filter_tokens(rare_ids + stop_ids + en_stop_ids + frequent_ids)
    dictionary.compactify()
    dictionary.save('tweets.dict')
    
    corpus = TweetCorpus(file_name, dictionary)
    #corpora.MmCorpus.serialize('tweets_corpus.mm', corpus)
    gensim.matutils.MmWriter.write_corpus('tweets_corpus.mm', corpus, num_terms=len(dictionary))
    
    print "I've done my job!"
    
def estimate_lda():
    id2word = gensim.corpora.Dictionary.load('stemmed2.dict')
    mm = gensim.corpora.MmCorpus('tweets_corpus.mm')
    
    print "About to run LDA estimation"
    #lda = gensim.models.ldamodel.LdaModel(corpus=mm, id2word=id2word, num_topics=100, update_every=0, passes=1, distributed=True)
    lda = gensim.models.ldamodel.LdaModel(corpus=mm, id2word=id2word, num_topics=100, update_every=1, chunksize=20000, passes=4, distributed=True)
    lda.save("tweets_lda")
    print "I've done my job!"

if __name__ == '__main__':
    
    if sys.argv[1] == 'topics':
        con = psycopg2.connect("host=192.168.25.33 dbname='tweets' user='tweets' password='zxc123'")
        
        id2word = gensim.corpora.Dictionary.load('small.dict')
        lda = LdaModel.load('small.lda')
        
        save_tweets_topics(lda, id2word, con, sys.argv[2])
    elif sys.argv[1] == 'infra':
        create_infra(sys.argv[2])
        
    elif sys.argv[1] == 'lda':
        estimate_lda()