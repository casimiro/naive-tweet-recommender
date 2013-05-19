#include "userprofile.h"
#include <QtSql/QtSql>
#include <iostream>
#include <vector>

namespace casimiro {
    
UserProfile::UserProfile(long _userId, ConceptMapPtr _profile, std::tm _start, std::tm _end):
    m_userId(_userId),
    m_profile(_profile),
    m_start(_start),
    m_end(_end),
    m_profileType(HASHTAG)
{
}

UserProfile::~UserProfile()
{
}

UserProfilePtr UserProfile::getHashtagProfile(long _userId, std::tm _start, std::tm _end)
{
    ConceptMap conceptMap;
    
    char s[19];
    char e[19];
    s[18] = '\0';
    s[18] = '\0';
    strftime(s, 19, "%Y-%m-%d %H:%M", &_start);
    strftime(e, 19, "%Y-%m-%d %H:%M", &_end);
    
    QSqlQuery query;
    query.prepare("SELECT content FROM tweet WHERE user_id = :uid AND creation_time >= :start AND creation_time <= :end AND content LIKE '%#%'");
    query.bindValue(":uid", (qlonglong) _userId);
    query.bindValue(":start", s);
    query.bindValue(":end", e);
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

    for(auto it = conceptMap.begin(); it != conceptMap.end(); it++)
        it->second = it->second / sum;
    
    UserProfilePtr up = std::make_shared<UserProfile>(_userId, std::make_shared<ConceptMap>(conceptMap), _start, _end);
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

TweetProfileVectorPtr UserProfile::getCandidateTweets(std::tm _start, std::tm _end)
{
    QSqlQuery query;
    TweetProfileVector tweetProfiles;

    char s[19];
    char e[19];
    s[18] = '\0';
    s[18] = '\0';
    strftime(s, 19, "%Y-%m-%d %H:%M", &_start);
    strftime(e, 19, "%Y-%m-%d %H:%M", &_end);

    if(m_profileType == HASHTAG)
    {
        query.prepare(
            "SELECT id, t.content, t.creation_time FROM tweet as t, relationship as r"
            " WHERE t.user_id = r.followed_id AND r.follower_id = :uid "
            "AND t.creation_time >= :start AND t.creation_time <= :end"
        );
        query.bindValue(":uid", (qlonglong) m_userId);
        query.bindValue(":start", s);
        query.bindValue(":end", e);
        query.exec();
        while(query.next())
        {
            long tweetId = query.value(0).toLongLong();
            QString content = query.value(1).toString();
            QDateTime creationTime = query.value(2).toDateTime();
            std::string dateString = creationTime.toString("yyyy-MM-dd hh:mm:ss").toStdString();
            std::tm date;
            strptime(dateString.c_str(), "%Y-%m-%d %H:%M:%S", &date);
            tweetProfiles.push_back(TweetProfile::getHashtagProfile(tweetId, date, content));
        }
    }
    return std::make_shared<TweetProfileVector>(tweetProfiles);
}

}
