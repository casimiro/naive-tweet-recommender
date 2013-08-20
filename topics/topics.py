import sys
import re
import logging
import psycopg2
import gensim
from gensim import corpora
from gensim.models.ldamodel import LdaModel
from ptstemmer.implementations.OrengoStemmer import OrengoStemmer
from ptstemmer.support import PTStemmerUtilities
import codecs

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
    for line in open(file_name, "r", buffering=8092000):
        vals = line.split()
        words = vals[5:]
        bow = dictionary.doc2bow(words)
        topics = lda[bow]
        data = ""
        for t in topics:
            data +=  str(t[0]) + ":" + str(t[1]) + " "
        data = data[:-1]
        
        retweeted = vals[4]
        if retweeted == "\N":
            retweeted = 0
            
        bulk.append((vals[0], vals[1] + ' ' + vals[2], vals[3], retweeted, data))
        if len(bulk) >= 50000:
            logging.info("Persisting bulk %s of size %s" % (run, len(bulk)))
            cur.executemany("INSERT INTO tweet_topics (id, creation_time, user_id, retweeted, topics) VALUES (%s,%s,%s,%s,%s) ", bulk)
            con.commit()
            logging.info("Persisted")
            bulk = []
            run += 1

def create_infra(file_name):
    tweets = Tweets(file_name)
    dictionary = corpora.Dictionary(tweets)
    stop_ids = [dictionary.token2id[stopword] for stopword in open("stopwords.txt") if stopword.strip() in dictionary.token2id]
    once_ids = [tokenid for tokenid, docfreq in dictionary.dfs.iteritems() if docfreq == 1]
    dictionary.filter_tokens(once_ids)
    dictionary.compactify()
    dictionary.save_as_text('dictionary')
    
    corpus = TweetCorpus(file_name, dictionary)
    corpora.MmCorpus.serialize('tweets_corpus.mm', corpus)
    
    print "I've done my job!"
    
def estimate_lda():
    id2word = gensim.corpora.Dictionary.load_from_text('dictionary')
    mm = gensim.corpora.MmCorpus('tweets_corpus.mm')
    
    print "About to run LDA estimation"
    lda = gensim.models.ldamodel.LdaModel(corpus=mm, id2word=id2word, num_topics=100, update_every=1, chunksize=50000, passes=1)
    lda.save("tweets_lda")
    print "I've done my job!"

if __name__ == '__main__':
    
    if sys.argv[1] == 'topics':
        con = psycopg2.connect("host=192.168.25.2 dbname='tweetsbr2' user='tweetsbr' password='zxc123'")
        
        id2word = gensim.corpora.Dictionary.load_from_text('dictionary')
        lda = LdaModel.load('tweets_lda')
        
        save_tweets_topics(lda, id2word, con, sys.argv[2])
    elif sys.argv[1] == 'infra':
        create_infra(sys.argv[2])
        
    elif sys.argv[1] == 'lda':
        estimate_lda()