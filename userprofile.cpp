#include "userprofile.h"
#include <iostream>
#include <vector>
#include <boost/regex.hpp>
#include <algorithm>
#include <ctime>
#include "dateutils.h"

namespace casimiro {
    
UserProfile::UserProfile(PqConnectionPtr _con, long _userId, ConceptMapPtr _profile, std::tm _start, std::tm _end, ProfileType _profileType, std::string _sqlQuery):
    m_con(_con),
    m_userId(_userId),
    m_profile(_profile),
    m_start(_start),
    m_end(_end),
    m_profileType(_profileType),
    m_sqlQuery(_sqlQuery)
{
}

UserProfile::~UserProfile()
{
}

void UserProfile::buildConceptMap(pqxx::result &_rows, std::string _pattern)
{
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

            if(m_profile->find(found) == m_profile->end())
                m_profile->insert(std::make_pair(found, 0));
            (*m_profile)[found] += 1;

            sum++;

            start = match[0].second;
        }
    }

    if(m_profile->begin() == m_profile->end())
        throw EmptyProfileException();

    for(auto it = m_profile->begin(); it != m_profile->end(); it++)
        it->second = it->second / sum;
}

void UserProfile::loadProfile()
{
    pqxx::nontransaction t(*m_con);
    m_con->prepare("pl", m_sqlQuery);
    pqxx::result rows = t.prepared("pl")(m_userId)(DateUtils::TmToString(m_start))(DateUtils::TmToString(m_end)).exec();

    if(m_profileType == BAG_OF_WORDS)
        buildConceptMap(rows, "\\w{3,}");
    else if(m_profileType == HASHTAG)
        buildConceptMap(rows, "#\\w+");

    t.commit();
}

UserProfilePtr UserProfile::getBagOfWordsProfile(PqConnectionPtr _con, long _userId, std::tm _start, std::tm _end, bool _social)
{
    std::string query = "SELECT content FROM tweet WHERE user_id = $1 AND creation_time >= $2 AND creation_time <= $3";
    if(_social)
        query = "SELECT content FROM tweet,relationship WHERE (user_id = $1 OR user_id = followed_id) AND follower_id = $1 AND creation_time >= $2 AND creation_time <= $3";

    UserProfilePtr up = std::make_shared<UserProfile>(_con, _userId, std::make_shared<ConceptMap>(), _start, _end, BAG_OF_WORDS, query);
    return up;
}

UserProfilePtr UserProfile::getHashtagProfile(PqConnectionPtr _con, long _userId, std::tm _start, std::tm _end, bool _social)
{
    std::string query = "SELECT content FROM tweet WHERE user_id = $1 AND creation_time >= $2 AND creation_time <= $3 AND content LIKE '%#%'";
    if(_social)
        query = "SELECT content FROM tweet,relationship WHERE (user_id = $1 OR user_id = followed_id) AND follower_id = $1 AND creation_time >= $2 AND creation_time <= $3 AND content LIKE '%#%'";

    UserProfilePtr up = std::make_shared<UserProfile>(_con, _userId, std::make_shared<ConceptMap>(), _start, _end, HASHTAG, query);
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

    if(m_profileType == HASHTAG)
    {
        m_con->prepare("ct",
            "SELECT id, content, creation_time FROM tweet,relationship "
            "WHERE user_id = followed_id AND follower_id = $1 "
            "AND creation_time >= $2 AND creation_time <= $3 AND content LIKE '%#%' ORDER BY creation_time ASC"
        );
        pqxx::result rows = t.prepared("ct")(m_userId)(DateUtils::TmToString(_start))(DateUtils::TmToString(_end)).exec();
        for(auto row : rows)
        {
            long tweetId = row["id"].as<long>();
            std::string content = row["content"].c_str();
            std::tm creationTime = DateUtils::StringToTm(row["creation_time"].c_str());
            tweetProfiles.push_back(TweetProfile::getHashtagProfile(tweetId, creationTime, content));
        }
    }
    else if(m_profileType == BAG_OF_WORDS)
    {
        m_con->prepare("ct",
            "SELECT id, content, creation_time FROM tweet,relationship "
            "WHERE user_id = followed_id AND follower_id = $1 "
            "AND creation_time >= $2 AND creation_time <= $3 ORDER BY creation_time ASC"
        );
        pqxx::result rows = t.prepared("ct")(m_userId)(DateUtils::TmToString(_start))(DateUtils::TmToString(_end)).exec();
        for(auto row : rows)
        {
            long tweetId = row["id"].as<long>();
            std::string content = row["content"].c_str();
            std::tm creationTime = DateUtils::StringToTm(row["creation_time"].c_str());
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

    pqxx::result rows = t.prepared("gr")(m_userId)(DateUtils::TmToString(_start))(DateUtils::TmToString(_end)).exec();

    for(auto row : rows)
    {
        std::tm creationTime = DateUtils::StringToTm(row["creation_time"].c_str());
        retweets.push_back(std::make_pair(creationTime, row["retweeted"].as<long>()));
    }

    t.commit();
    return std::make_shared<RetweetVector>(retweets);
}


}
