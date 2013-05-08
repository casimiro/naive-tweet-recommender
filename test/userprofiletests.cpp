#include <gtest/gtest.h>
#include <ctime>
#include "../profiles.h"
#include <QtSql/QtSql>

namespace casimiro {

class ProfileTestCase : public ::testing::Test {
protected:
    ProfileTestCase()
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    
        db.setDatabaseName("gao.db.sqlite");
        db.open();
        
        QSqlQuery query;
        query.exec("DROP TABLE tweets_sample");
        query.exec("CREATE TABLE 'tweets_sample' ("
            "'id' bigint(20) NOT NULL primary key,"
            "'userId' bigint(20) DEFAULT NULL,"
            "'username' varchar(255) NOT NULL,"
            "'content' varchar(255) NOT NULL,"
            "'creationTime' timestamp NULL DEFAULT NULL,"
            "'favorite' tinyint(1) NOT NULL DEFAULT '0',"
            "'replyToPostId' bigint(20) DEFAULT NULL,"
            "'replyToUsername' varchar(255) DEFAULT NULL,"
            "'retweetedFromPostId' bigint(20) DEFAULT NULL,"
            "'retweetedFromUsername' varchar(255) DEFAULT NULL,"
            "'retweetCount' int(11) DEFAULT NULL,"
            "'latitude' double DEFAULT NULL,"
            "'longitude' double DEFAULT NULL,"
            "'placeCountry' varchar(100) DEFAULT NULL,"
            "'placeCountryCode' varchar(20) DEFAULT NULL,"
            "'placeStreetAddress' varchar(255) DEFAULT NULL,"
            "'placeURL' varchar(255) DEFAULT NULL,"
            "'placeGeometryType' varchar(255) DEFAULT NULL,"
            "'placeName' varchar(255) DEFAULT NULL,"
            "'placeFullName' varchar(255) DEFAULT NULL,"
            "'placeId' varchar(255) DEFAULT NULL,"
            "'source' varchar(255) DEFAULT NULL)"
        );
        query.exec("DROP TABLE semanticsTweetsEntity");
        query.exec("CREATE TABLE 'semanticsTweetsEntity' ("
            "'userId' int(11) DEFAULT NULL,"
            "'tweetId' bigint(20) NOT NULL,"
            "'type' varchar(255) DEFAULT NULL,"
            "'typeURI' varchar(255) DEFAULT NULL,"
            "'name' varchar(255) DEFAULT NULL,"
            "'uri' varchar(255) DEFAULT NULL,"
            "'relevance' double DEFAULT NULL,"
            "'creationTime' timestamp NULL DEFAULT NULL,"
            "primary key(tweetid, uri))"
        );
        
        // Populating tweets_sample
        query.exec("INSERT INTO tweets_sample (id, userId, username, content, creationTime) VALUES (1, 1, 'testuser1', 'bla bla asd', '2012-01-01 09:05:00')");
        query.exec("INSERT INTO tweets_sample (id, userId, username, content, creationTime) VALUES (2, 1, 'testuser1', 'bla bla 43', '2012-01-01 09:10:00')");
        query.exec("INSERT INTO tweets_sample (id, userId, username, content, creationTime) VALUES (3, 1, 'testuser1', 'bla asd', '2012-01-01 09:15:00')");
        query.exec("INSERT INTO tweets_sample (id, userId, username, content, creationTime) VALUES (4, 2, 'testuser2', 'bla bla', '2012-01-01 09:00:00')");
        query.exec("INSERT INTO tweets_sample (id, userId, username, content, creationTime) VALUES (5, 2, 'testuser2', 'bla bla fgh', '2012-01-01 09:03:00')");
        query.exec("INSERT INTO tweets_sample (id, userId, username, content, creationTime) VALUES (6, 1, 'testuser1', 'bla asd', '2012-01-01 10:15:00')");
        
        // Populating semanticsTweetsEntity
        query.exec(
            "INSERT INTO semanticsTweetsEntity (userId, tweetId, type, typeURI, name, uri, relevance, creationTime)"
            "VALUES (1, 1, 'URL', 'http://s.opencalais.com/1/type/em/e/URL', 'http://on.cnn.com/cj8ioi', '70503365', 0.714, '2012-01-01 09:05:00')"
        );
        query.exec(
            "INSERT INTO semanticsTweetsEntity (userId, tweetId, type, typeURI, name, uri, relevance, creationTime)"
            "VALUES (1, 1, 'URL', 'http://s.opencalais.com/1/type/em/e/URL', 'http://nyti.ms/9Sndzv', 'c6a014a7', 0.45, '2012-01-01 09:05:00')"
        );
        query.exec(
            "INSERT INTO semanticsTweetsEntity (userId, tweetId, type, typeURI, name, uri, relevance, creationTime)" 
            "VALUES (1, 2, 'URL', 'http://s.opencalais.com/1/type/em/e/URL', 'http://on.cnn.com/cj8ioi', '70503365', 0.714, '2012-01-01 09:10:00')"
        );
        query.exec(
            "INSERT INTO semanticsTweetsEntity (userId, tweetId, type, typeURI, name, uri, relevance, creationTime)" 
            "VALUES (1, 3, 'URL', 'http://s.opencalais.com/1/type/em/e/URL', 'http://nyti.ms/9Sndzv', 'c6a014a7', 0.45, '2012-01-01 09:15:00')"
        );
        query.exec(
            "INSERT INTO semanticsTweetsEntity (userId, tweetId, type, typeURI, name, uri, relevance, creationTime)" 
            "VALUES (1, 6, 'URL', 'http://s.opencalais.com/1/type/em/e/URL', 'http://nyti.ms/9Sndzv', 'c6a014a7', 0.45, '2012-01-01 10:15:00')"
        );
        
        query.exec(
            "INSERT INTO semanticsTweetsEntity (userId, tweetId, type, typeURI, name, uri, relevance, creationTime)" 
            "VALUES (2, 4, 'URL', 'http://s.opencalais.com/1/type/em/e/URL', 'http://on.cnn.com/cj8ioi', '70503365', 0.714, '2012-01-01 09:00:00')"
        );
        query.exec(
            "INSERT INTO semanticsTweetsEntity (userId, tweetId, type, typeURI, name, uri, relevance, creationTime)" 
            "VALUES (2, 4, 'URL', 'http://s.opencalais.com/1/type/em/e/URL', 'http://nyti.ms/9Sndzv', 'c6a014a7', 0.45, '2012-01-01 09:00:00')"
        );
        
        query.exec(
            "INSERT INTO semanticsTweetsEntity (userId, tweetId, type, typeURI, name, uri, relevance, creationTime)" 
            "VALUES (2, 5, 'URL', 'http://s.opencalais.com/1/type/em/e/URL', 'http://on.cnn.com/cj8ioi', '70503365', 0.714, '2012-01-01 09:03:00')"
        );
        
        query.exec("DROP TABLE news");
        query.exec(
            "CREATE TABLE news ("
            "id bigint(20) NOT NULL,"
            "source varchar(255)  DEFAULT NULL,"
            "category varchar(255) DEFAULT NULL,"
            "url text,"
            "title varchar(400),"
            "description text,"
            "newscontent longtext,"
            "publish_date datetime DEFAULT NULL,"
            "update_date datetime DEFAULT NULL,"
            "crawl_date timestamp NULL DEFAULT CURRENT_TIMESTAMP,"
            "PRIMARY KEY (id))"
        );

        query.exec("DROP TABLE semanticsNewsEntity");
        query.exec(
            "CREATE TABLE semanticsNewsEntity ("
            "newsId bigint(20) NOT NULL,"
            "type varchar(255) DEFAULT NULL,"
            "typeURI varchar(255) DEFAULT NULL,"
            "name varchar(255) DEFAULT NULL,"
            "uri varchar(255) DEFAULT NULL,"
            "relevance double DEFAULT NULL,"
            "publish_date timestamp DEFAULT NULL,"
            "PRIMARY KEY(newsId, uri))"
        );

        query.exec(
            "INSERT INTO news (id, source, category, url, title, description, newscontent, publish_date, update_date, crawl_date)"
            "VALUES ('8', 'http://newsrss.bbc.co.uk/rss/', 'sport', 'http://news.bbc.co.uk/9040797.stm', 'FC Utrecht v Liverpool', 'Liverpool captain Steven...', NULL, '2010-09-29 13:26:48', NULL, '2010-09-30 13:07:02')"
        );

        query.exec(
            "INSERT INTO semanticsNewsEntity (newsId, type, typeURI, name, uri, relevance, publish_date)"
            "VALUES ('8', 'Person', 'http://Person', 'Jamie Carragher', '6339ea3a-9cf1-3ccc-8ae9-77a56b6997fb', '0.383', '2010-09-29 08:26:48')"
        );

        query.exec(
            "INSERT INTO semanticsNewsEntity (newsId, type, typeURI, name, uri, relevance, publish_date)"
            "VALUES ('8', 'Person', 'http://Person', 'Raul Meireles', '0928195e-36b1-3dc7-9dff-80dc5ff07a88', '0.658', '2010-09-29 08:26:48')"
        );

        query.exec(
            "INSERT INTO semanticsNewsEntity (newsId, type, typeURI, name, uri, relevance, publish_date)"
            "VALUES ('8', 'Person', 'http://Person', 'Ricky van Wolfswinkel', '4ded0348-ec05-3763-85c6-efa8693af9c3', '0.648', '2010-09-29 08:26:48')"
        );

        query.exec(
            "INSERT INTO semanticsNewsEntity (newsId, type, typeURI, name, uri, relevance, publish_date)"
            "VALUES ('8', 'Person', 'http://Person', 'Barry Maguire', 'a19b6044-9349-389d-9e96-91e62b8d17ef', '0.274', '2010-09-29 08:26:48')"
        );
        
        query.exec(
            "INSERT INTO news (id, source, category, url, title, description, newscontent, publish_date, update_date, crawl_date)"
            "VALUES ('9', 'http://newsrss.bbc.co.uk/rss/', 'sport', 'http://news.bbc.co.uk/9040797.stm', 'FC Utrecht v Liverpool', 'Liverpool captain Steven...', NULL, '2010-09-30 13:26:48', NULL, '2010-09-30 13:07:02')"
        );

        query.exec(
            "INSERT INTO semanticsNewsEntity (newsId, type, typeURI, name, uri, relevance, publish_date)"
            "VALUES ('9', 'Person', 'http://Person', 'Jamie Carragher', '6339ea3a-9cf1-3ccc-8ae9-77a56b6997fb', '0.383', '2010-09-30 08:26:48')"
        );

        query.exec(
            "INSERT INTO semanticsNewsEntity (newsId, type, typeURI, name, uri, relevance, publish_date)"
            "VALUES ('9', 'Person', 'http://Person', 'Raul Meireles', '0928195e-36b1-3dc7-9dff-80dc5ff07a88', '0.658', '2010-09-30 08:26:48')"
        );

        query.exec(
            "INSERT INTO semanticsNewsEntity (newsId, type, typeURI, name, uri, relevance, publish_date)"
            "VALUES ('9', 'Person', 'http://Person', 'Ricky van Wolfswinkel', '4ded0348-ec05-3763-85c6-efa8693af9c3', '0.648', '2010-09-30 08:26:48')"
        );

        query.exec(
            "INSERT INTO semanticsNewsEntity (newsId, type, typeURI, name, uri, relevance, publish_date)"
            "VALUES ('9', 'Person', 'http://Person', 'Barry Maguire', 'a19b6044-9349-389d-9e96-91e62b8d17ef', '0.274', '2010-09-30 08:26:48')"
        );
    }
};

TEST_F(ProfileTestCase, UserProfileLoading)
{
    long int userId = 1;
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
    
    UserProfilePtr up = UserProfile::getUserProfile(userId, start, end);
    
    ASSERT_NE(nullptr, up);
    ASSERT_NE(up->getProfile(), nullptr);
    ASSERT_EQ(up->getUserId(), 1);
    ASSERT_EQ(2, up->getProfile()->size());
    ASSERT_EQ(0.5, up->getProfile()->at("70503365"));
    ASSERT_EQ(0.5, up->getProfile()->at("c6a014a7"));
    
    up = UserProfile::getUserProfile(2, start, end);
    
    ASSERT_NE(nullptr, up);
    ASSERT_NE(up->getProfile(), nullptr);
    ASSERT_EQ(up->getUserId(), 2);
    ASSERT_EQ(2, up->getProfile()->size());
    ASSERT_NEAR(0.66, up->getProfile()->at("70503365"), 0.01);
    ASSERT_NEAR(0.33, up->getProfile()->at("c6a014a7"), 0.01);
}

TEST_F(ProfileTestCase, NewsProfileLoadingById)
{
    
    int id = 8;
    NewsProfilePtr up = NewsProfile::getNewsProfile(id); 
    
    ASSERT_NE(nullptr, up);
    ASSERT_NE(nullptr, up->getProfile());
    
    ASSERT_EQ(0.25, up->getProfile()->at("6339ea3a-9cf1-3ccc-8ae9-77a56b6997fb"));
    ASSERT_EQ(0.25, up->getProfile()->at("0928195e-36b1-3dc7-9dff-80dc5ff07a88"));
    ASSERT_EQ(0.25, up->getProfile()->at("4ded0348-ec05-3763-85c6-efa8693af9c3"));
    ASSERT_EQ(0.25, up->getProfile()->at("a19b6044-9349-389d-9e96-91e62b8d17ef"));
    
}

TEST_F(ProfileTestCase, NewsProfileLoadingByPeriod)
{
    
    std::tm start;
    std::tm end;
    
    start.tm_year = 2010 - 1900;
    start.tm_mon = 8;
    start.tm_mday = 29;
    start.tm_hour = 0;
    start.tm_min = 0;
    
    end.tm_year = 2010 - 1900;
    end.tm_mon = 8;
    end.tm_mday = 30;
    end.tm_hour = 23;
    end.tm_min = 59;
    
    NewsProfileListPtr up = NewsProfile::getNewsProfilesBetween(start, end);
    
    ASSERT_NE(nullptr, up);
    ASSERT_EQ(2, up->size());
    
}


}