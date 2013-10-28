select EXTRACT(EPOCH FROM avg(t2.creation_time - t1.creation_time)), tt.topic from tweet as t1, tweet t2, tweet_topic as tt 
where t2.retweeted = t1.id and tt.tweet_id = t2.id GROUP BY tt.topic 
