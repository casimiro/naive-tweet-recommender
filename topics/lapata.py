import psycopg2
import numpy as np
from sklearn.metrics.pairwise import cosine_similarity
from sklearn.preprocessing import normalize
from scipy.sparse import lil_matrix, csr_matrix
from scipy.io import mmwrite, mmread
from datetime import timedelta
from collections import defaultdict

def ndcg(ranking, relevant_items):
    ndcg = 0.0
    perfect_dcg = 0.0
    for i in range(len(ranking)):
        power = 1 if ranking[i] in relevant_items else 0
        ndcg += (pow(2.0, power) - 1) / np.log2(2 + i)
        
        #perfect ndcg
        power = 1 if i < len(relevant_items) else 0
        perfect_dcg += (pow(2.0, power) - 1) / np.log2(2 + i)
    return ndcg / perfect_dcg

def mean_average_precision(ranking, relevant_items):
    _map = 0.0
    precision = 0
    for i in range(len(ranking)):
        if ranking[i] in relevant_items:
            precision += 1
        relevant = 1 if ranking[i] in relevant_items else 0
        _map += precision * relevant
        
    return _map / len(relevant_items)

class YanRecommender(object):

    def __init__(self, con):
        self.con = con
        self._load_matrix_U()
        self._sigma = 0.15
        self._lambda = 0.6

    def _load_candidates(self, start, end):
        cur = self.con.cursor()
        cur.execute("""
            SELECT tt.tweet_id, tt.topic, tt.topic_value
            FROM tweet_topic AS tt LEFT JOIN tweet AS t ON tt.tweet_id = t.id
            WHERE creation_time BETWEEN '%s' AND '%s' AND user_id in
            (SELECT followed_id FROM relationship WHERE follower_id = %s) ORDER BY tt.tweet_id
            """ % (start, end, self.user_id) )
        rows = cur.fetchall()

        self._candidate_ids = {} # map the tweet position in matrices given tweet id
        self._inv_candidate_ids = {} # map the tweet position in matrices given tweet id
        self._D = []
        d_row = np.zeros(100)
        
        current_tweet_id = rows[0][0]
        for row in rows:
            if row[0] != current_tweet_id:
                self._candidate_ids[current_tweet_id] = len(self._D)
                self._inv_candidate_ids[len(self._D)] = current_tweet_id
                self._D.append(d_row)
                d_row = np.zeros(100)
                current_tweet_id = row[0]
            d_row[row[1]] = row[2]
        
        self._candidate_ids[current_tweet_id] = len(self._D)
        self._inv_candidate_ids[len(self._D)] = current_tweet_id
        self._D.append(d_row)

        self._M = cosine_similarity(self._D, self._D)
        self._r = np.dot(self._D,self._t)
        
        # Loading users that have retweeted the candidates
        self._MU = lil_matrix((len(self._candidate_ids.keys()), len(self._user_ids.keys())), dtype=np.dtype(float))
        self._UM = lil_matrix((len(self._user_ids.keys()), len(self._candidate_ids.keys())), dtype=np.dtype(float))
        
        sql = "SELECT retweeted,user_id FROM tweet WHERE retweeted IN ("
        for id in self._candidate_ids.keys():
            sql += str(id) + ','
        sql = sql[:-1]
        sql += ") ORDER BY id, user_id ASC"
        
        cur.execute(sql)
        rows = cur.fetchall()
        
        for row in rows:
            self._MU[self._candidate_ids[row[0]], self._user_ids[row[1]]] = 1
            self._UM[self._user_ids[row[1]], self._candidate_ids[row[0]]] = 1
            
        self._MU = self._MU.tocsr()
        self._UM = self._UM.tocsr()
        
        normalize(self._MU, 'l1', copy=False)
        normalize(self._UM, 'l1', copy=False)

    def _load_matrix_U(self):
        self._user_ids = {}
        i = 0
        for user_id in open('user_ids'):
            self._user_ids[int(user_id.strip())] = i
            i += 1

        try:
            self._U = mmread("U.data")
        except:
            self._U = lil_matrix((len(self._user_ids), len(self._user_ids)), dtype=np.int8)

            cur = self.con.cursor()
            cur.execute("SELECT follower_id, followed_id FROM relationship ORDER BY follower_id, followed_id ASC")
            rows = cur.fetchall()

            for row in rows:
                if self._user_ids.has_key(row[0]) and self._user_ids.has_key(row[1]):
                    i = self._user_ids[row[0]]
                    j = self._user_ids[row[1]]
                    self._U[i,j] = 1

            self._U = self._U.tocsr()
            mmwrite("U.data",self._U)

    def set_user(self, user_id):
        self.user_id = user_id
        cur = self.con.cursor()
        cur.execute(""" SELECT topic, topic_value FROM user_topic WHERE user_id = %s  """ % user_id)
        rows = cur.fetchall()
        self._t = np.zeros(100)
        self._p = np.zeros(len(self._user_ids.keys()))
        
        cur.execute(""" SELECT tweets_count FROM twitter_user WHERE id = %s """ % user_id)
        row = cur.fetchone()
        tweets_count = float(row[0])
        
        for row in rows:
            self._t[row[0]] = row[1]
            
        cur.execute("""
            SELECT t.user_id, count(t.*) FROM tweet AS t LEFT JOIN tweet AS t2 ON t2.retweeted = t.id
            WHERE t2.creation_time < '2013-05-01' AND t2.user_id = %s GROUP BY t.user_id
            """ % user_id )
        
        rows = cur.fetchall()
        
        for row in rows:
            user_pos = self._user_ids[row[0]]
            self._p[user_pos] = float(row[1]) / tweets_count
        


    def produce_recommendations(self, start, end):
        # load user's topic preference vector

        # load user's p vector (preference over other users)

        # load candidates tweets
        #   Build M matrix
        #   Build r vector (r = tD)
        #   Build U matrix
        #   Build UM matrix
        #   Build MU matrix
        self._load_candidates(start, end)
        
        self._m = np.tile(1.0 / len(self._candidate_ids.keys()), len(self._candidate_ids.keys()))
        self._u = np.tile(1.0 / len(self._user_ids.keys()), len(self._user_ids.keys()))
        
        diag_r = csr_matrix( (self._r, (np.arange(len(self._r)), np.arange(len(self._r)))), shape=(len(self._r),len(self._r)), dtype=np.dtype(float))
        diag_p = csr_matrix( (self._u, (np.arange(len(self._u)), np.arange(len(self._u)))), shape=(len(self._u),len(self._u)), dtype=np.dtype(float))
        
        diff = 1
        while diff > 0.001:
            # updating M
            self._M = (1 - self._sigma) * ( self._M * self._m )  + ( self._sigma * self._r )
            
            aux_m = (1 - self._lambda) * ( diag_r.dot( self._M ).transpose().dot(self._m) ) + self._lambda * ( self._UM.transpose().dot(self._u) )
            aux_m = aux_m/ np.linalg.norm(aux_m)
            
            aux_u = (1 - self._sigma) * ( diag_p.dot( self._U ).transpose().dot(self._u) ) + self._sigma * ( self._MU.transpose().dot(self._m) )
            aux_u = aux_u / np.linalg.norm(aux_u)
            
            diff_m = np.absolute(self._m - aux_m)
            diff_u = np.absolute(self._u - aux_u)
            
            diff = min(np.min(diff_m), np.min(diff_u))
            
            self._m = aux_m
            self._u = aux_u
        
        top50 = self._m.argsort()[-50:][::-1]
        return [ self._inv_candidate_ids[x] for x in top50 ]
    
    def evaluate(self, start):
        dates_retweets = defaultdict(set)
        
        cur = self.con.cursor()
        cur.execute("""
            SELECT date(creation_time), retweeted FROM tweet WHERE creation_time > '%s' AND user_id = %s AND retweeted IS NOT NULL 
            """ % (start, self.user_id))
        rows = cur.fetchall()
        for row in rows:
            dates_retweets[row[0]].add(row[1])
        
        one_day = timedelta(days=1)
        all_ndcg = 0.0
        all_map = 0.0
        for date in dates_retweets.keys():
            recommendations = self.produce_recommendations(date, date + one_day)
            all_ndcg += ndcg(recommendations, dates_retweets[date])
            all_map += mean_average_precision(recommendations, dates_retweets[date])
        all_ndcg = all_ndcg / len(dates_retweets.keys())
        all_map = all_map / len(dates_retweets.keys())
        return (all_ndcg,all_map)
            


if __name__ == '__main__':
    con = psycopg2.connect("host=192.168.25.33 dbname='tweets' user='tweets' password='zxc123'")
    yan = YanRecommender(con)
    all_ndcg = 0.0
    all_map = 0.0
    for user_id in open("users_good"):
        yan.set_user(int(user_id))
        results = yan.evaluate('2013-05-01')
        print "%s   %s  %s" % (user_id, results[0], results[1])
        all_ndcg += results[0]
        all_map += results[1]
    print "Final result: nDCG: %s     MAP: %s" % (all_ndcg, all_map)
