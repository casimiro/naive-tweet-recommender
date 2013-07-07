#include "userprofile.h"
#include <iostream>
#include <vector>
#include <boost/regex.hpp>
#include <algorithm>
#include <ctime>

namespace casimiro {
    
UserProfile::UserProfile(PqConnectionPtr _con, long _userId, ConceptMapPtr _profile, std::tm _start, std::tm _end, ProfileType _profileType):
    m_con(_con),
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

ConceptMap UserProfile::buildConceptMap(pqxx::result &_rows, std::string _pattern)
{
    ConceptMap conceptMap;

    boost::regex rx(_pattern);
    boost::match_results<std::string::const_iterator> match;
    std::string::const_iterator start;

    float sum = 0;
    for(auto row : _rows)
    {
        std::string content = row["content"].c_str();
        start = content.cbegin();
        while(boost::regex_search(start, content.cend(), match, rx, boost::match_default))
        {
            std::string found(match[0].first, match[0].second);
            std::transform(found.begin(), found.end(), found.begin(), ::tolower);

            if(conceptMap.find(found) == conceptMap.end())
                conceptMap.insert(std::make_pair(found, 0));
            conceptMap[found] += 1;

            sum++;

            start = match[0].second;
        }
    }

    if(conceptMap.begin() == conceptMap.end())
        throw EmptyProfileException();

    for(auto it = conceptMap.begin(); it != conceptMap.end(); it++)
        it->second = it->second / sum;

    return conceptMap;
}

UserProfilePtr UserProfile::getBagOfWordsProfile(PqConnectionPtr _con, long _userId, std::tm _start, std::tm _end, bool _social)
{
    pqxx::nontransaction t(*_con);

    char s[19];
    char e[19];
    strftime(s, 19, "%Y-%m-%d %H:%M", &_start);
    strftime(e, 19, "%Y-%m-%d %H:%M", &_end);

    if(_social)
        _con->prepare("pbow", "SELECT content FROM tweet,relationship WHERE (user_id = $1 OR user_id = followed_id) AND follower_id = $1 AND creation_time >= $2 AND creation_time <= $3");
    else
        _con->prepare("pbow", "SELECT content FROM tweet WHERE user_id = $1 AND creation_time >= $2 AND creation_time <= $3");

    pqxx::result rows = t.prepared("pbow")(_userId)(s)(e).exec();

    UserProfilePtr up = std::make_shared<UserProfile>(_con, _userId, std::make_shared<ConceptMap>(buildConceptMap(rows, "\\w{3,}")), _start, _end, BAG_OF_WORDS);

    t.commit();
    return up;
}

UserProfilePtr UserProfile::getHashtagProfile(PqConnectionPtr _con, long _userId, std::tm _start, std::tm _end, bool _social)
{
    pqxx::nontransaction t(*_con);

    char s[19];
    char e[19];
    strftime(s, 19, "%Y-%m-%d %H:%M", &_start);
    strftime(e, 19, "%Y-%m-%d %H:%M", &_end);

    if(_social)
        _con->prepare("ph", "SELECT content FROM tweet,relationship WHERE (user_id = $1 OR user_id = followed_id) AND follower_id = $1 AND creation_time >= $2 AND creation_time <= $3 AND content LIKE '%#%'");
    else
        _con->prepare("ph", "SELECT content FROM tweet WHERE user_id = $1 AND creation_time >= $2 AND creation_time <= $3 AND content LIKE '%#%'");

    pqxx::result rows = t.prepared("ph")(_userId)(s)(e).exec();
    t.commit();

    UserProfilePtr up = std::make_shared<UserProfile>(_con, _userId, std::make_shared<ConceptMap>(buildConceptMap(rows, "#\\w+")), _start, _end, HASHTAG);
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
    pqxx::nontransaction t(*m_con);
    TweetProfileVector tweetProfiles;
    char s[19];
    char e[19];
    strftime(s, 19, "%Y-%m-%d %H:%M", &_start);
    strftime(e, 19, "%Y-%m-%d %H:%M", &_end);

    if(m_profileType == HASHTAG)
    {
        m_con->prepare("ct",
            "SELECT id, content, creation_time FROM tweet "
            "WHERE user_id IN (select followed_id from relationship WHERE follower_id = $1)"
            "AND creation_time >= $2 AND creation_time <= $3 AND content LIKE '%#%' ORDER BY creation_time ASC"
        );
        pqxx::result rows = t.prepared("ct")(m_userId)(std::string(s))(std::string(e)).exec();
        for(auto row : rows)
        {
            long tweetId = row["id"].as<long>();
            std::string content = row["content"].c_str();
            std::tm creationTime;
            strptime(row["creation_time"].c_str(), "%Y-%m-%d %H:%M", &creationTime);

            tweetProfiles.push_back(TweetProfile::getHashtagProfile(tweetId, creationTime, content));
        }
    }
    else if(m_profileType == BAG_OF_WORDS)
    {
        m_con->prepare("ct",
            "SELECT id, content, creation_time FROM tweet "
            "WHERE user_id IN (select followed_id from relationship WHERE follower_id = $1) "
            "AND creation_time >= $2 AND creation_time <= $3 ORDER BY creation_time ASC"
        );
        pqxx::result rows = t.prepared("ct")(m_userId)(s)(e).exec();
        for(auto row : rows)
        {
            long tweetId = row["id"].as<long>();
            std::string content = row["content"].c_str();
            std::tm creationTime;
            strptime(row["creation_time"].c_str(), "%Y-%m-%d %H:%M", &creationTime);
            tweetProfiles.push_back(TweetProfile::getBagOfWordsProfile(tweetId, creationTime, content));
        }
    }
    t.commit();
    return std::make_shared<TweetProfileVector>(tweetProfiles);
}

RetweetVectorPtr UserProfile::getRetweets(std::tm _start, std::tm _end)
{
    RetweetVector retweets;
    pqxx::nontransaction t(*m_con);
    char s[19];
    char e[19];
    strftime(s, 19, "%Y-%m-%d %H:%M", &_start);
    strftime(e, 19, "%Y-%m-%d %H:%M", &_end);
    
    if(m_profileType == HASHTAG)
    {
        m_con->prepare("gr",
        "SELECT creation_time, retweeted FROM tweet WHERE user_id = $1 "
        "AND retweeted IS NOT NULL AND creation_time >= $2 AND creation_time <= $3 AND content LIKE '%#%' "
        "ORDER BY creation_time ASC"
        );
    }
    else if (m_profileType == BAG_OF_WORDS)
    {
        m_con->prepare("gr",
            "SELECT creation_time, retweeted FROM tweet WHERE user_id = $1 "
            "AND retweeted IS NOT NULL AND creation_time >= $2 AND creation_time <= $3 "
            "ORDER BY creation_time ASC"
        );
    }

    pqxx::result rows = t.prepared("gr")(m_userId)(s)(e).exec();

    for(auto row : rows)
    {
        std::tm creationTime;
        strptime(row["creation_time"].c_str(), "%Y-%m-%d %X", &creationTime);
        retweets.push_back(std::make_pair(creationTime, row["retweeted"].as<long>()));
    }

    t.commit();
    return std::make_shared<RetweetVector>(retweets);
}

}
