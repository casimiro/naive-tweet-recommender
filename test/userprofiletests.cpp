#include <gtest/gtest.h>
#include <ctime>
#include "../userprofile.h"
#include <pqxx/pqxx>

namespace casimiro {

class ProfileTestCase : public ::testing::Test {
protected:
    PqConnectionPtr m_con;

    ProfileTestCase()
    {
        m_con = std::make_shared<pqxx::connection>("postgresql://tweetsbr:zxc123@localhost:5432/tweetsbrtest");
        pqxx::work t(*m_con);

        t.exec("DROP TABLE IF EXISTS tweet");
        t.exec(
            "CREATE TABLE tweet ("
            "id bigint NOT NULL,"
            "content text,"
            "creation_time timestamp,"
            "user_id bigint,"
            "retweeted bigint,"
            "CONSTRAINT tweet_pk PRIMARY KEY (id ))"
        );

        t.exec("INSERT INTO tweet (id, user_id, creation_time, retweeted, content) VALUES (1, 1, '2012-01-01 09:05:00', null, '#USP bla bla')");
        t.exec("INSERT INTO tweet (id, user_id, creation_time, retweeted, content) VALUES (2, 1, '2012-01-01 09:10:00', null, 'bla #google bla')");
        t.exec("INSERT INTO tweet (id, user_id, creation_time, retweeted, content) VALUES (3, 1, '2012-01-01 09:15:00', null, 'bla bla #linux')");

        t.exec("INSERT INTO tweet (id, user_id, creation_time, retweeted, content) VALUES (7, 1, '2012-01-01 11:00:00', 4, 'RT: #UNICAMP bla bla bla')");
        t.exec("INSERT INTO tweet (id, user_id, creation_time, retweeted, content) VALUES (8, 1, '2012-01-01 11:03:00', 5, 'RT: bla bla #apple asdf')");

        t.exec("INSERT INTO tweet (id, user_id, creation_time, retweeted, content) VALUES (4, 2, '2012-01-01 09:00:00', null, '#UNICAMP bla bla bla')");
        t.exec("INSERT INTO tweet (id, user_id, creation_time, retweeted, content) VALUES (5, 2, '2012-01-01 09:03:00', null, 'bla bla #apple asdf')");
        t.exec("INSERT INTO tweet (id, user_id, creation_time, retweeted, content) VALUES (6, 2, '2012-01-01 10:15:00', null, 'bla bla kkkk #mac')");

        t.exec("DROP TABLE IF EXISTS relationship");
        t.exec(
            "CREATE TABLE relationship"
            "("
            "follower_id bigint NOT NULL,"
            "followed_id bigint NOT NULL,"
            "CONSTRAINT relationship_id PRIMARY KEY (follower_id , followed_id )"
            ")"
        );
        t.exec("INSERT INTO relationship (follower_id, followed_id) VALUES (1, 2)");
        t.commit();
    }
};

TEST_F(ProfileTestCase, UserProfileLoading)
{
    std::tm start;
    std::tm end;
    strptime("2012-01-01 09:00:00", "%Y-%m-%d %H:%M", &start);
    strptime("2012-01-01 10:00:00", "%Y-%m-%d %H:%M", &end);

    UserProfilePtr up = UserProfile::getHashtagProfile(m_con, 1, start, end, false);
    
    ASSERT_NE(nullptr, up);
    ASSERT_NE(up->getProfile(), nullptr);
    ASSERT_EQ(up->getUserId(), 1);
    ASSERT_EQ(3, up->getProfile()->size());
    ASSERT_NEAR(0.33, up->getProfile()->at("#usp"), 0.01);
    ASSERT_NEAR(0.33, up->getProfile()->at("#google"), 0.01);
    ASSERT_NEAR(0.33, up->getProfile()->at("#linux"), 0.01);
    
    up = UserProfile::getHashtagProfile(m_con, 2, start, end, false);
    
    ASSERT_NE(nullptr, up);
    ASSERT_NE(up->getProfile(), nullptr);
    ASSERT_EQ(up->getUserId(), 2);
    ASSERT_EQ(2, up->getProfile()->size());
    ASSERT_NEAR(0.50, up->getProfile()->at("#unicamp"), 0.01);
    ASSERT_NEAR(0.50, up->getProfile()->at("#apple"), 0.01);
}

TEST_F(ProfileTestCase, GetCandidateTweets)
{
    std::tm start;
    std::tm end;
    strptime("2012-01-01 09:00:00", "%Y-%m-%d %H:%M", &start);
    strptime("2012-01-01 10:00:00", "%Y-%m-%d %H:%M", &end);

    UserProfilePtr up = UserProfile::getHashtagProfile(m_con, 1, start, end, false);
    auto tweetProfiles = up->getCandidateTweets(start, end);

    ASSERT_EQ(2, tweetProfiles->size());
    ASSERT_EQ(tweetProfiles->at(0)->getProfile()->size(), 1);
    ASSERT_EQ(tweetProfiles->at(1)->getProfile()->size(), 1);

    ASSERT_NEAR(1.0, tweetProfiles->at(0)->getProfile()->at("#unicamp"), 0.1);
    ASSERT_NEAR(1.0, tweetProfiles->at(1)->getProfile()->at("#apple"), 0.1);
}

TEST_F(ProfileTestCase, GetRetweets)
{
    std::tm start;
    std::tm end;
    strptime("2012-01-01 11:00:00", "%Y-%m-%d %H:%M", &start);
    strptime("2012-01-01 12:00:00", "%Y-%m-%d %H:%M", &end);

    UserProfilePtr up = UserProfile::getHashtagProfile(m_con, 1, start, end, false);
    auto retweets = up->getRetweets(start, end);

    ASSERT_EQ(2, retweets->size());
    ASSERT_EQ(4, retweets->at(0).second);
    ASSERT_EQ(0, retweets->at(0).first.tm_min);

    ASSERT_EQ(5, retweets->at(1).second);
    ASSERT_EQ(3, retweets->at(1).first.tm_min);
}

}
