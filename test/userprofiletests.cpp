#include <gtest/gtest.h>
#include <ctime>
#include "../userprofile.h"
#include "../newsprofile.h"
#include <QtSql/QtSql>

namespace casimiro {

class ProfileTestCase : public ::testing::Test {
protected:
    ProfileTestCase()
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    
        db.setDatabaseName("casimiro.db.sqlite");
        db.open();
        
        QSqlQuery query;
        query.exec("DROP TABLE tweet");
        query.exec(
            "CREATE TABLE 'tweet' ("
            "id bigint NOT NULL,"
            "content text,"
            "creation_time timestamp,"
            "user_id bigint,"
            "retweeted bigint,"
            "CONSTRAINT tweet_pk PRIMARY KEY (id ))"
        );

        // Populating tweets_sample
        query.exec("INSERT INTO tweet (id, user_id, creation_time, retweeted, content) VALUES (1, 1, '2012-01-01 09:05:00', null, '#USP bla bla')");
        query.exec("INSERT INTO tweet (id, user_id, creation_time, retweeted, content) VALUES (2, 1, '2012-01-01 09:10:00', null, 'bla #google bla')");
        query.exec("INSERT INTO tweet (id, user_id, creation_time, retweeted, content) VALUES (3, 1, '2012-01-01 09:15:00', null, 'bla bla #linux')");

        query.exec("INSERT INTO tweet (id, user_id, creation_time, retweeted, content) VALUES (4, 2, '2012-01-01 09:00:00', null, '#UNICAMP bla bla bla')");
        query.exec("INSERT INTO tweet (id, user_id, creation_time, retweeted, content) VALUES (5, 2, '2012-01-01 09:03:00', null, 'bla bla #apple asdf')");
        query.exec("INSERT INTO tweet (id, user_id, creation_time, retweeted, content) VALUES (6, 2, '2012-01-01 10:15:00', null, 'bla bla kkkk #mac')");

    }
};

TEST_F(ProfileTestCase, UserProfileLoading)
{
    std::tm start;
    std::tm end;
    
    start.tm_year = 2012 - 1900;
    start.tm_mon = 0;
    start.tm_mday = 1;
    start.tm_hour = 9;
    start.tm_min = 0;
    
    end.tm_year = 2012 - 1900;
    end.tm_mon = 0;
    end.tm_mday = 1;
    end.tm_hour = 10;
    end.tm_min = 0;
    
    UserProfilePtr up = UserProfile::getHashtagProfile(1, start, end);
    
    ASSERT_NE(nullptr, up);
    ASSERT_NE(up->getProfile(), nullptr);
    ASSERT_EQ(up->getUserId(), 1);
    ASSERT_EQ(3, up->getProfile()->size());
    ASSERT_NEAR(0.33, up->getProfile()->at("#usp"), 0.01);
    ASSERT_NEAR(0.33, up->getProfile()->at("#google"), 0.01);
    ASSERT_NEAR(0.33, up->getProfile()->at("#linux"), 0.01);
    
    up = UserProfile::getHashtagProfile(1, start, end);
    
    ASSERT_NE(nullptr, up);
    ASSERT_NE(up->getProfile(), nullptr);
    ASSERT_EQ(up->getUserId(), 2);
    ASSERT_EQ(3, up->getProfile()->size());
    ASSERT_NEAR(0.33, up->getProfile()->at("#unicamp"), 0.01);
    ASSERT_NEAR(0.33, up->getProfile()->at("#apple"), 0.01);
    ASSERT_NEAR(0.33, up->getProfile()->at("#mac"), 0.01);
}

}
