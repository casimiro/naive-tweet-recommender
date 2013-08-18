import sys
import re
import logging
import psycopg2
import gensim
from gensim.models.ldamodel import LdaModel
from ptstemmer.implementations.OrengoStemmer import OrengoStemmer
from ptstemmer.support import PTStemmerUtilities
import codecs

logging.basicConfig(format='%(asctime)s : %(levelname)s : %(message)s', level=logging.INFO)

class TweetCorpus(object):
    def __iter__(self):
        for line in open('lda_data'):
            # assume there's one document per line, tokens separated by whitespace
            yield dictionary.doc2bow(line.split()[1:])
            
            
def save_tweets_topics(lda, dictionary, con, file_name, stemmer):
    cur = con.cursor()
        
    rx = re.compile(r"\w{3,}", re.UNICODE)
    bulk = []
    run = 0
    for line in codecs.open(file_name, "r", "utf-8", buffering=8092000):
        vals = line.split(u'\t')
        finds = rx.findall(vals[1])
        stems = [stemmer.getWordStem(x) for x in finds]
        words = stems
        bow = dictionary.doc2bow(words)
        topics = lda[bow]
        data = ""
        for t in topics:
            data +=  str(t[0]) + ":" + str(t[1]) + " "
        data = data[:-1]
        
        retweeted = vals[4]
        if retweeted == "\N":
            retweeted = 0
            
        bulk.append((vals[0], vals[2], vals[3], retweeted, data))
        if len(bulk) >= 500000:
            logging.info("Persisting bulk %s of size %s" % (run, len(bulk)))
            cur.executemany("INSERT INTO tweet_topics (id, creation_time, user_id, retweeted, topics) VALUES (%s,%s,%s,%s,%s) ", bulk)
            con.commit()
            logging.info("Persisted")
            bulk = []
            run += 1

if __name__ == '__main__':
    con = psycopg2.connect("host=192.168.25.2 dbname='tweetsbr2' user='tweetsbr' password='zxc123'")
    
    id2word = gensim.corpora.Dictionary.load_from_text('tweets_dict.txt')
    lda = LdaModel.load('lda')
    
    stemmer = OrengoStemmer()
    stemmer.enableCaching(1000)
    stemmer.ignore(PTStemmerUtilities.fileToSet("namedEntitiesU.txt"))
    
    save_tweets_topics(lda, id2word, con, sys.argv[1], stemmer)