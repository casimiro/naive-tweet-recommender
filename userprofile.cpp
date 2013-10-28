#include "userprofile.h"
#include <iostream>
#include <vector>
#include <boost/regex.hpp>
#include <algorithm>
#include <sstream>
#include "dateutils.h"
#include <boost/algorithm/string.hpp>

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
                (*m_profile)[found] += 2;
                sum += 2;
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

void UserProfile::buildTopicsConceptMap(pqxx::result _rows)
{
    float sum = 0;
    float val = 0;
    
    std::vector<std::string> pairs;
    size_t colonPos;
    std::string topic;
    
    for(auto row : _rows)
    {
        pairs.clear();
        std::string line = row["topics"].c_str();
        boost::split(pairs, line, boost::is_any_of(" "));
        for (auto pair : pairs)
        {
            colonPos = pair.find(":");
            topic = pair.substr(0, colonPos).c_str();
            val = atof(pair.substr(colonPos+1, pair.size() - colonPos).c_str());

            if(row["user_id"].as<long>() == m_userId)
            {
                (*m_profile)[topic] += 2*val;
                sum += 2*val;
            }
            else
            {
                (*m_profile)[topic] += val;
                sum += val;
            }

        }
    }
    
    for(auto it = m_profile->begin(); it != m_profile->end(); it++)
        it->second = it->second / sum;
}

void UserProfile::loadProfile()
{
    pqxx::nontransaction t(*m_con);
    pqxx::result rows = t.exec(m_sqlQuery);

    switch(m_profileType)
    {
        case BAG_OF_WORDS:
            buildConceptMap(rows, "\\w{3,}");
            break;
        case HASHTAG:
            buildConceptMap(rows, "#\\w+");
            break;
        case TOPICS:
            buildTopicsConceptMap(rows);
    }
    

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
        query = "SELECT content, user_id FROM tweet WHERE user_id in ((SELECT followed_id FROM relationship WHERE follower_id="+sUserId+") UNION (SELECT GREATEST(0,"+sUserId+"))) "
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
        query = "SELECT content, user_id FROM tweet WHERE user_id in ((SELECT followed_id FROM relationship WHERE follower_id="+sUserId+") UNION (SELECT GREATEST(0,"+sUserId+"))) "
                "AND creation_time >= '"+to_iso_extended_string(_start)+"' AND creation_time <= '"+to_iso_extended_string(_end)+"' AND content LIKE '%#%'";
    }

    UserProfilePtr up = std::make_shared<UserProfile>(_con, _userId, std::make_shared<ConceptMap>(), _start, _end, HASHTAG, query);
    return up;
}

UserProfilePtr UserProfile::getTopicsProfile(PqConnectionPtr _con, long _userId, ptime _start, ptime _end, bool _social)
{
    std::stringstream ss;
    ss << _userId;
    std::string sUserId = ss.str();

    std::string query = "SELECT topics, user_id FROM tweet_topics WHERE user_id = "+sUserId+" "
            "AND topics != '' AND creation_time >= '"+to_iso_extended_string(_start)+"' AND creation_time <= '"+to_iso_extended_string(_end)+"'";
    if(_social)
    {
        query = "SELECT topics, user_id FROM tweet_topics WHERE user_id in ((SELECT followed_id FROM relationship WHERE follower_id="+sUserId+") UNION (SELECT GREATEST(0,"+sUserId+"))) "
                "AND topics != '' AND creation_time >= '"+to_iso_extended_string(_start)+"' AND creation_time <= '"+to_iso_extended_string(_end)+"'";
    }

    UserProfilePtr up = std::make_shared<UserProfile>(_con, _userId, std::make_shared<ConceptMap>(), _start, _end, TOPICS, query);
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

struct {
    bool operator()(TweetProfilePtr a, TweetProfilePtr b)
    {
        return a->getTweetId() < b->getTweetId();
    }
} TweetProfileLess;

TweetProfileVectorPtr UserProfile::getCandidateTweets(const ptime &_start, const ptime &_end)
{
    pqxx::nontransaction t(*m_con);
    TweetProfileVector tweetProfiles;
    std::stringstream ss;
    ss << m_userId;
    std::string sUserId = ss.str();
    
    std::string start = to_iso_extended_string(_start);
    std::string end = to_iso_extended_string(_end);

    switch(m_profileType)
    {
        case HASHTAG:
        {
            pqxx::result rows = t.exec("SELECT id, content, creation_time FROM tweet,relationship "
                "WHERE user_id = followed_id AND follower_id = "+sUserId+" "
                "AND creation_time >= '"+start+"' AND creation_time <= '"+end+"' AND content LIKE '%#%'");
            
            for(auto row : rows)
            {
                long tweetId = row["id"].as<long>();
                std::string content = row["content"].c_str();
                ptime creationTime = time_from_string(row["creation_time"].c_str());
                tweetProfiles.push_back(TweetProfile::getHashtagProfile(tweetId, creationTime, content));
            }
            break;
        }
        case BAG_OF_WORDS:
        {
            pqxx::result rows = t.exec("SELECT id, content, creation_time FROM tweet "
                "WHERE user_id IN (select followed_id FROM relationship WHERE follower_id = "+sUserId+") "
                "AND creation_time >= '"+start+"' AND creation_time <= '"+end+"'");
            for(auto row : rows)
            {
                long tweetId = row["id"].as<long>();
                std::string content = row["content"].c_str();
                ptime creationTime = time_from_string(row["creation_time"].c_str());
                tweetProfiles.push_back(TweetProfile::getBagOfWordsProfile(tweetId, creationTime, content));
            }
            break;
        }
        case TOPICS:
        {
            pqxx::result rows = t.exec("SELECT id, topics, creation_time FROM tweet_topics "
                "WHERE topics != '' AND user_id IN (select followed_id FROM relationship WHERE follower_id = "+sUserId+") "
                "AND creation_time >= '"+start+"' AND creation_time <= '"+end+"'");
            for(auto row : rows)
            {
                long tweetId = row["id"].as<long>();
                std::string topics = row["topics"].c_str();
                ptime creationTime = time_from_string(row["creation_time"].c_str());
                tweetProfiles.push_back(TweetProfile::getTopicProfile(tweetId, creationTime, topics));
            }
            break;
        }
    }
    t.commit();

    std::sort(tweetProfiles.begin(), tweetProfiles.end(), TweetProfileLess);
    return std::make_shared<TweetProfileVector>(tweetProfiles);
}

RetweetVectorPtr UserProfile::getRetweets(const ptime &_start, const ptime &_end)
{
    RetweetVector retweets;
    pqxx::nontransaction t(*m_con);
        
    switch(m_profileType)
    {
        case HASHTAG:
            m_con->prepare("gr",
            "SELECT creation_time, retweeted FROM tweet WHERE user_id = $1 "
            "AND retweeted IS NOT NULL AND creation_time >= $2 AND creation_time <= $3 AND content LIKE '%#%' "
            "ORDER BY creation_time ASC"
            );
            break;
        
        case BAG_OF_WORDS:
            m_con->prepare("gr",
                "SELECT creation_time, retweeted FROM tweet WHERE user_id = $1 "
                "AND retweeted IS NOT NULL AND creation_time >= $2 AND creation_time <= $3 "
                "ORDER BY creation_time ASC"
            );
            break;
    
        case TOPICS:
            m_con->prepare("gr",
                "SELECT creation_time, retweeted FROM tweet_topics WHERE user_id = $1 "
                "AND retweeted IS NOT NULL AND retweeted != 0 AND creation_time >= $2 AND creation_time <= $3 "
                "ORDER BY creation_time ASC"
            );
            break;
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
