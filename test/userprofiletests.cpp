#include <gtest/gtest.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "userprofile.h"
#include <pqxx/pqxx>
#include "dateutils.h"
#include "evaluation.h"

namespace casimiro {

using namespace boost::posix_time;

class ProfileTestCase : public ::testing::Test {
protected:
    std::string m_stringConnection;
    PqConnectionPtr m_con;

    ProfileTestCase()
    {
        m_stringConnection = "postgresql://tweetsbr:zxc123@localhost:5432/tweetsbrtest";
        m_con = std::make_shared<pqxx::connection>(m_stringConnection);
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

        /** training data **/
        t.exec("INSERT INTO tweet (id, user_id, creation_time, retweeted, content) VALUES (1, 1, '2012-01-01 09:05:00', null, '#USP bla bla')");
        t.exec("INSERT INTO tweet (id, user_id, creation_time, retweeted, content) VALUES (2, 1, '2012-01-01 09:10:00', null, 'bla #google bla')");
        t.exec("INSERT INTO tweet (id, user_id, creation_time, retweeted, content) VALUES (3, 1, '2012-01-01 09:15:00', null, 'bla bla #linux')");

        t.exec("INSERT INTO tweet (id, user_id, creation_time, retweeted, content) VALUES (7, 1, '2012-01-01 11:00:00', 4, 'RT: #UNICAMP bla bla bla')");
        t.exec("INSERT INTO tweet (id, user_id, creation_time, retweeted, content) VALUES (8, 1, '2012-01-01 11:03:00', 5, 'RT: #Unicamp bla bla #apple asdf')");

        t.exec("INSERT INTO tweet (id, user_id, creation_time, retweeted, content) VALUES (4, 2, '2012-01-01 09:00:00', null, '#UNICAMP bla bla bla')");
        t.exec("INSERT INTO tweet (id, user_id, creation_time, retweeted, content) VALUES (5, 2, '2012-01-01 09:03:00', null, 'bla bla #apple asdf')");
        t.exec("INSERT INTO tweet (id, user_id, creation_time, retweeted, content) VALUES (6, 2, '2012-01-01 10:15:00', null, 'bla bla kkkk #mac')");

        t.exec("INSERT INTO tweet (id, user_id, creation_time, retweeted, content) VALUES (9, 3, '2012-01-01 09:15:00', null, 'dilma na presidencia #politica')");
        t.exec("INSERT INTO tweet (id, user_id, creation_time, retweeted, content) VALUES (10, 3, '2012-01-01 09:05:00', null, 'inflacao alta #economia')");

        /** test data **/
        t.exec("INSERT INTO tweet (id, user_id, creation_time, retweeted, content) VALUES (11, 2, '2012-01-07 09:00:00', null, 'trabalho #unicamp')");
        t.exec("INSERT INTO tweet (id, user_id, creation_time, retweeted, content) VALUES (12, 2, '2012-01-07 09:00:00', null, 'trabalho #som')");
        t.exec("INSERT INTO tweet (id, user_id, creation_time, retweeted, content) VALUES (13, 2, '2012-01-07 09:05:00', null, 'trabalhando #apple')");

        t.exec("INSERT INTO tweet (id, user_id, creation_time, retweeted, content) VALUES (14, 3, '2012-01-07 09:10:00', null, '#economia em baixa')");
        t.exec("INSERT INTO tweet (id, user_id, creation_time, retweeted, content) VALUES (15, 4, '2012-01-07 09:15:00', null, 'corinthians ganha do #palmeiras')");

        t.exec("INSERT INTO tweet (id, user_id, creation_time, retweeted, content) VALUES (16, 1, '2012-01-07 09:07:00', 11, 'trabalho #unicamp')");
        t.exec("INSERT INTO tweet (id, user_id, creation_time, retweeted, content) VALUES (17, 1, '2012-01-07 09:20:00', 14, '#economia em baixa')");

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
        t.exec("INSERT INTO relationship (follower_id, followed_id) VALUES (1, 3)");
        t.commit();
    }
};

TEST_F(ProfileTestCase, UserProfileLoading)
{
    ptime start = time_from_string("2012-01-01 09:00:00");
    ptime end = time_from_string("2012-01-01 10:00:00");

    UserProfilePtr up = UserProfile::getHashtagProfile(m_con, 1, start, end, false);
    up->loadProfile();
    
    ASSERT_NE(nullptr, up);
    ASSERT_NE(up->getProfile(), nullptr);
    ASSERT_EQ(up->getUserId(), 1);
    ASSERT_EQ(3, up->getProfile()->size());
    ASSERT_NEAR(0.33, up->getProfile()->at("#usp"), 0.01);
    ASSERT_NEAR(0.33, up->getProfile()->at("#google"), 0.01);
    ASSERT_NEAR(0.33, up->getProfile()->at("#linux"), 0.01);
    
    up = UserProfile::getHashtagProfile(m_con, 2, start, end, false);
    up->loadProfile();
    
    ASSERT_NE(nullptr, up);
    ASSERT_NE(up->getProfile(), nullptr);
    ASSERT_EQ(up->getUserId(), 2);
    ASSERT_EQ(2, up->getProfile()->size());
    ASSERT_NEAR(0.50, up->getProfile()->at("#unicamp"), 0.01);
    ASSERT_NEAR(0.50, up->getProfile()->at("#apple"), 0.01);

}

TEST_F(ProfileTestCase, UserProfileSocialLoading)
{
    ptime start = time_from_string("2012-01-01 09:00:00");
    ptime end = time_from_string("2012-01-01 10:00:00");

    UserProfilePtr up = UserProfile::getHashtagProfile(m_con, 1, start, end, true);
    up->loadProfile();
    ASSERT_NE(nullptr, up);
    ASSERT_NE(up->getProfile(), nullptr);
    ASSERT_EQ(up->getUserId(), 1);
    ASSERT_EQ(7, up->getProfile()->size());
    ASSERT_NEAR(0.20, up->getProfile()->at("#usp"), 0.01);
    ASSERT_NEAR(0.20, up->getProfile()->at("#google"), 0.01);
    ASSERT_NEAR(0.20, up->getProfile()->at("#linux"), 0.01);
    ASSERT_NEAR(0.10, up->getProfile()->at("#unicamp"), 0.01);
    ASSERT_NEAR(0.10, up->getProfile()->at("#apple"), 0.01);
    ASSERT_NEAR(0.10, up->getProfile()->at("#politica"), 0.01);
    ASSERT_NEAR(0.10, up->getProfile()->at("#economia"), 0.01);
}

TEST_F(ProfileTestCase, GetCandidateTweets)
{
    ptime start = time_from_string("2012-01-07 09:00:00");
    ptime end = time_from_string("2012-01-07 10:00:00");

    UserProfilePtr up = UserProfile::getHashtagProfile(m_con, 1, start, end, false);
    up->loadProfile();
    auto tweetProfiles = up->getCandidateTweets(start, end);

    ASSERT_EQ(4, tweetProfiles->size());
    ASSERT_EQ(tweetProfiles->at(0)->getProfile()->size(), 1);
    ASSERT_EQ(tweetProfiles->at(1)->getProfile()->size(), 1);

    ASSERT_NEAR(1.0, tweetProfiles->at(0)->getProfile()->at("#unicamp"), 0.1);
    ASSERT_NEAR(1.0, tweetProfiles->at(1)->getProfile()->at("#som"), 0.1);
}

TEST_F(ProfileTestCase, GetRetweets)
{
    ptime start = time_from_string("2012-01-01 11:00:00");
    ptime end = time_from_string("2012-01-01 12:00:00");

    UserProfilePtr up = UserProfile::getHashtagProfile(m_con, 1, start, end, false);
    auto retweets = up->getRetweets(start, end);

    ASSERT_EQ(2, retweets->size());
    ASSERT_EQ(4, retweets->at(0).second);
    ASSERT_EQ(0, retweets->at(0).first.time_of_day().minutes());

    ASSERT_EQ(5, retweets->at(1).second);
    ASSERT_EQ(3, retweets->at(1).first.time_of_day().minutes());
}

TEST_F(ProfileTestCase, EvaluationRun)
{
    auto users = std::make_shared<LongVector>();
    users->push_back(1);
    Evaluation eval(m_stringConnection, 
                    users,
                    time_from_string("2012-01-01 00:00:00"),
                    time_from_string("2012-01-01 11:59:59"),
                    time_from_string("2012-01-07 00:00:00"),
                    time_from_string("2012-01-07 11:59:59"),
                    HASHTAG_EVAL,
                    false);

    EvaluationResults result = eval.run();

    ASSERT_NEAR(0.625, result.generalResult().mrr, 0.001);
    ASSERT_NEAR(0.625, result.resultMap()[1].mrr, 0.001);
}

}
