#include "userprofile.h"
#include <iostream>
#include <vector>
#include <boost/regex.hpp>
#include <algorithm>
#include <sstream>
#include "dateutils.h"

namespace casimiro {
    
UserProfile::UserProfile(PqConnectionPtr _con, long _userId, ConceptMapPtr _profile, ptime _start, ptime _end, ProfileType _profileType, std::string _sqlQuery):
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

            if(row["user_id"].as<long>() == m_userId)
            {
                (*m_profile)[found] += 100;
                sum += 100;
            }
            else
            {
                (*m_profile)[found] += 1;
                sum++;
            }


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
    pqxx::result rows = t.exec(m_sqlQuery);

    if(m_profileType == BAG_OF_WORDS)
        buildConceptMap(rows, "\\w{3,}");
    else if(m_profileType == HASHTAG)
        buildConceptMap(rows, "#\\w+");

    t.commit();
}

UserProfilePtr UserProfile::getBagOfWordsProfile(PqConnectionPtr _con, long _userId, ptime _start, ptime _end, bool _social)
{
    std::stringstream ss;
    ss << _userId;
    std::string sUserId = ss.str();

    std::string query = "SELECT content, user_id FROM tweet WHERE user_id = "+sUserId+" "
            "AND creation_time >= '"+to_iso_extended_string(_start)+"' AND creation_time <= '"+to_iso_extended_string(_end)+"'";
    if(_social)
    {
        query = "SELECT content, user_id FROM tweet WHERE user_id in (SELECT followed_id FROM relationship WHERE follower_id="+sUserId+" UNION SELECT 1 FROM twitter_user) "
                "AND creation_time >= '"+to_iso_extended_string(_start)+"' AND creation_time <= '"+to_iso_extended_string(_end)+"'";
    }

    UserProfilePtr up = std::make_shared<UserProfile>(_con, _userId, std::make_shared<ConceptMap>(), _start, _end, BAG_OF_WORDS, query);
    return up;
}

UserProfilePtr UserProfile::getHashtagProfile(PqConnectionPtr _con, long _userId, ptime _start, ptime _end, bool _social)
{
    std::stringstream ss;
    ss << _userId;
    std::string sUserId = ss.str();

    std::string query = "SELECT content, user_id FROM tweet WHERE user_id = "+sUserId+" "
            "AND creation_time >= '"+to_iso_extended_string(_start)+"' AND creation_time <= '"+to_iso_extended_string(_end)+"' AND content LIKE '%#%'";
    if(_social)
    {
        query = "SELECT content, user_id FROM tweet WHERE user_id in (SELECT followed_id FROM relationship WHERE follower_id="+sUserId+" UNION SELECT 1 FROM twitter_user) "
                "AND creation_time >= '"+to_iso_extended_string(_start)+"' AND creation_time <= '"+to_iso_extended_string(_end)+"' AND content LIKE '%#%'";
    }

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

TweetProfileVectorPtr UserProfile::getCandidateTweets(ptime _start, ptime _end)
{
    pqxx::nontransaction t(*m_con);
    TweetProfileVector tweetProfiles;
    std::stringstream ss;
    ss << m_userId;
    std::string sUserId = ss.str();

    if(m_profileType == HASHTAG)
    {
        std::string query =
            "SELECT id, content, creation_time FROM tweet,relationship "
            "WHERE user_id = followed_id AND follower_id = "+sUserId+" "
            "AND creation_time >= '"+to_iso_extended_string(_start)+"' AND creation_time <= '"+to_iso_extended_string(_end)+"' AND content LIKE '%#%' ORDER BY creation_time ASC";
        pqxx::result rows = t.exec(query);
        for(auto row : rows)
        {
            long tweetId = row["id"].as<long>();
            std::string content = row["content"].c_str();
            ptime creationTime = time_from_string(row["creation_time"].c_str());
            tweetProfiles.push_back(TweetProfile::getHashtagProfile(tweetId, creationTime, content));
        }
    }
    else if(m_profileType == BAG_OF_WORDS)
    {
        std::string query =
            "SELECT id, content, creation_time FROM tweet,relationship "
            "WHERE user_id = followed_id AND follower_id = "+sUserId+" "
            "AND creation_time >= '"+to_iso_extended_string(_start)+"' AND creation_time <= '"+to_iso_extended_string(_end)+"' ORDER BY creation_time ASC";
        pqxx::result rows = t.exec(query);
        for(auto row : rows)
        {
            long tweetId = row["id"].as<long>();
            std::string content = row["content"].c_str();
            ptime creationTime = time_from_string(row["creation_time"].c_str());
            tweetProfiles.push_back(TweetProfile::getBagOfWordsProfile(tweetId, creationTime, content));
        }
    }
    t.commit();
    return std::make_shared<TweetProfileVector>(tweetProfiles);
}

RetweetVectorPtr UserProfile::getRetweets(ptime _start, ptime _end)
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

    pqxx::result rows = t.prepared("gr")(m_userId)(to_iso_extended_string(_start))(to_iso_extended_string(_end)).exec();

    for(auto row : rows)
    {
        ptime creationTime = time_from_string(row["creation_time"].c_str());
        retweets.push_back(std::make_pair(creationTime, row["retweeted"].as<long>()));
    }

    t.commit();
    return std::make_shared<RetweetVector>(retweets);
}


}
