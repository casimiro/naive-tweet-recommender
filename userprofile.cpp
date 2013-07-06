#include "userprofile.h"
#include <QtSql/QtSql>
#include <iostream>
#include <vector>

namespace casimiro {
    
UserProfile::UserProfile(long _userId, ConceptMapPtr _profile, QDateTime _start, QDateTime _end, ProfileType _profileType):
    m_userId(_userId),
    m_profile(_profile),
    m_start(_start),
    m_end(_end),
    m_profileType(_profileType)
{
}

UserProfile::~UserProfile()
{
}

UserProfilePtr UserProfile::getBagOfWordsProfile(long _userId, QDateTime _start, QDateTime _end)
{
    ConceptMap conceptMap;
    QSqlQuery query;
    query.prepare("SELECT content FROM tweet WHERE user_id = :uid AND creation_time >= :start AND creation_time <= :end");
    query.bindValue(":uid", (qlonglong) _userId);
    query.bindValue(":start", _start.toString("yyyy-MM-dd HH:mm:ss"));
    query.bindValue(":end", _end.toString("yyyy-MM-dd HH:mm:ss"));
    query.exec();

    float sum = 0;
    QRegExp rx("\\w{3,}");
    while(query.next())
    {
        QString content = query.value(0).toString();
        int pos = 0;
        while ((pos = rx.indexIn(content, pos)) != -1)
        {
            std::string match = rx.cap(0).toLower().toStdString();
            if(conceptMap.find(match) == conceptMap.end())
                conceptMap.insert(std::make_pair(match, 0));

            conceptMap[match] += 1;
            sum++;
            pos += rx.matchedLength();
        }
    }

    if(conceptMap.begin() == conceptMap.end())
        throw EmptyProfileException();

    for(auto it = conceptMap.begin(); it != conceptMap.end(); it++)
        it->second = it->second / sum;

    UserProfilePtr up = std::make_shared<UserProfile>(_userId, std::make_shared<ConceptMap>(conceptMap), _start, _end, BAG_OF_WORDS);
    return up;
}

UserProfilePtr UserProfile::getHashtagProfile(long _userId, QDateTime _start, QDateTime _end)
{
    ConceptMap conceptMap;
    QSqlQuery query;
    query.prepare("SELECT content FROM tweet WHERE user_id = :uid AND creation_time >= :start AND creation_time <= :end AND content LIKE '%#%'");
    query.bindValue(":uid", (qlonglong) _userId);
    query.bindValue(":start", _start.toString("yyyy-MM-dd HH:mm:ss"));
    query.bindValue(":end", _end.toString("yyyy-MM-dd HH:mm:ss"));
    query.exec();
    
    float sum = 0;
    QRegExp rx("#\\w+");
    while(query.next())
    {
        QString content = query.value(0).toString();
        int pos = 0;
        while ((pos = rx.indexIn(content, pos)) != -1)
        {
            std::string match = rx.cap(0).toLower().toStdString();
            if(conceptMap.find(match) == conceptMap.end())
                conceptMap.insert(std::make_pair(match, 0));

            conceptMap[match] += 1;
            sum++;
            pos += rx.matchedLength();
        }
    }

    if(conceptMap.begin() == conceptMap.end())
        throw EmptyProfileException();

    for(auto it = conceptMap.begin(); it != conceptMap.end(); it++)
        it->second = it->second / sum;
    
    UserProfilePtr up = std::make_shared<UserProfile>(_userId, std::make_shared<ConceptMap>(conceptMap), _start, _end, HASHTAG);
    return up;
}

double UserProfile::cosineSimilarity(ConceptMapPtr _profile)
{
    float aNorm,bNorm,dot;
    aNorm = bNorm = dot = 0;
    
    auto uIt = m_profile->begin();
    auto nIt = _profile->begin();
    
    auto uEnd = m_profile->end();
    auto nEnd = _profile->end();
    
    int compare;
    while (uIt != uEnd && nIt != nEnd)
    {
        compare = uIt->first.compare(nIt->first);
        if(compare == 0)
        {
            dot += uIt->second * nIt->second;
            uIt++;
            nIt++;
        }
        else if(compare < 0)
            uIt++;
        else if(compare > 0)
            nIt++;
    }
    
    for (uIt = m_profile->begin(); uIt != uEnd; uIt++)
        aNorm += pow(uIt->second, 2);
    aNorm = sqrt(aNorm);
    
    for (nIt = _profile->begin(); nIt != nEnd; nIt++)
        bNorm += pow(nIt->second, 2);
    bNorm = sqrt(bNorm);
    
    return dot / (aNorm * bNorm);
}

TweetProfileVectorPtr UserProfile::getCandidateTweets(QDateTime _start, QDateTime _end)
{
    QSqlQuery query;
    TweetProfileVector tweetProfiles;

    if(m_profileType == HASHTAG)
    {
        query.prepare(
            "SELECT id, content, creation_time FROM tweet "
            "WHERE user_id IN (select followed_id from relationship WHERE follower_id = :uid)"
            "AND creation_time >= :start AND creation_time <= :end AND content LIKE '%#%' ORDER BY creation_time ASC"
        );
        query.bindValue(":uid", (qlonglong) m_userId);
        query.bindValue(":start", _start.toString("yyyy-MM-dd HH:mm:ss"));
        query.bindValue(":end", _end.toString("yyyy-MM-dd HH:mm:ss"));
        query.exec();
        while(query.next())
        {
            long tweetId = query.value(0).toLongLong();
            QString content = query.value(1).toString();
            QDateTime creationTime = query.value(2).toDateTime();
            tweetProfiles.push_back(TweetProfile::getHashtagProfile(tweetId, creationTime, content));
        }
    }
    else if(m_profileType == BAG_OF_WORDS)
    {
        query.prepare(
            "SELECT id, content, creation_time FROM tweet "
            "WHERE user_id IN (select followed_id from relationship WHERE follower_id = :uid) "
            "AND creation_time >= :start AND creation_time <= :end ORDER BY creation_time ASC"
        );
        query.bindValue(":uid", (qlonglong) m_userId);
        query.bindValue(":start", _start.toString("yyyy-MM-dd HH:mm:ss"));
        query.bindValue(":end", _end.toString("yyyy-MM-dd HH:mm:ss"));
        query.exec();
        while(query.next())
        {
            long tweetId = query.value(0).toLongLong();
            QString content = query.value(1).toString();
            QDateTime creationTime = query.value(2).toDateTime();
            tweetProfiles.push_back(TweetProfile::getBagOfWordsProfile(tweetId, creationTime, content));
        }
    }
    return std::make_shared<TweetProfileVector>(tweetProfiles);
}

RetweetVectorPtr UserProfile::getRetweets(QDateTime _start, QDateTime _end)
{
    RetweetVector retweets;
    QSqlQuery query;
    
    if(m_profileType == HASHTAG)
    {
        query.prepare(
        "SELECT creation_time, retweeted FROM tweet WHERE user_id = :uid "
        "AND retweeted IS NOT NULL AND creation_time >= :start AND creation_time <= :end AND content LIKE '%#%' "
        "ORDER BY creation_time ASC"
        );
    } else if (m_profileType == BAG_OF_WORDS)
    {
        query.prepare(
            "SELECT creation_time, retweeted FROM tweet WHERE user_id = :uid "
            "AND retweeted IS NOT NULL AND creation_time >= :start AND creation_time <= :end "
            "ORDER BY creation_time ASC"
        );
    }
    query.bindValue(":uid", (qlonglong) m_userId);
    query.bindValue(":start", _start.toString("yyyy-MM-dd HH:mm:ss"));
    query.bindValue(":end", _end.toString("yyyy-MM-dd HH:mm:ss"));
    query.exec();

    while(query.next())
    {
        QDateTime creationTime = query.value(0).toDateTime();
        long retweetId = query.value(1).toLongLong();
        retweets.push_back(std::make_pair(creationTime, retweetId));
    }

    return std::make_shared<RetweetVector>(retweets);
}

}
