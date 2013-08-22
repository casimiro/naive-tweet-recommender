#include "tweetprofile.h"
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>

namespace casimiro {

TweetProfile::~TweetProfile()
{
}

TweetProfile::TweetProfile(long _id, ptime _publishDate, ConceptMapPtr _profile):
    m_id(_id),
    m_publishDateTime(_publishDate),
    m_profile(_profile)
{
}

TweetProfilePtr TweetProfile::buildProfile(long _id, ptime _publishDate, std::string _tweetContent, std::string _pattern)
{
    ConceptMap conceptMap;
    boost::regex rx(_pattern);
    boost::match_results<std::string::const_iterator> match;
    std::string::const_iterator start = _tweetContent.cbegin();
    float sum = 0;

    while(boost::regex_search(start, _tweetContent.cend(), match, rx, boost::match_default))
    {
        std::string found(match[0].first, match[0].second);
        std::transform(found.begin(), found.end(), found.begin(), ::tolower);

        if(conceptMap.find(found) == conceptMap.end())
            conceptMap.insert(std::make_pair(found, 0));
        conceptMap[found] += 1;

        sum += 1;
        start = match[0].second;
    }

    for(auto it = conceptMap.begin(); it != conceptMap.end(); it++)
        it->second = it->second / sum;

    return TweetProfilePtr(new TweetProfile(_id, _publishDate, std::make_shared<ConceptMap>(conceptMap)));
}

TweetProfilePtr TweetProfile::buildTopicProfile(long int _id, boost::posix_time::ptime _publishDate, std::string _topics)
{
    ConceptMap conceptMap;
    std::vector<std::string> pairs;
    size_t colonPos;
    std::string topic;
    float val, sum = 0;
    
    boost::split(pairs, _topics, boost::is_any_of(" "));
    for(auto pair : pairs)
    {
        colonPos = pair.find(":");
        topic = pair.substr(0, colonPos).c_str();
        val = atof(pair.substr(colonPos+1, pair.size() - colonPos).c_str());
        conceptMap[topic] = val;
        sum += val;
    }
    for(auto it = conceptMap.begin(); it != conceptMap.end(); it++)
        it->second = it->second / sum;
    
    return TweetProfilePtr(new TweetProfile(_id, _publishDate, std::make_shared<ConceptMap>(conceptMap)));
}


TweetProfilePtr TweetProfile::getBagOfWordsProfile(long _id, ptime _publishDate, std::string _tweetContent)
{
    return buildProfile(_id, _publishDate, _tweetContent, "\\w{3,}");
}


TweetProfilePtr TweetProfile::getHashtagProfile(long _id, ptime _publishDate, std::string _tweetContent)
{
    return buildProfile(_id, _publishDate, _tweetContent, "#\\w+");
}

TweetProfilePtr TweetProfile::getTopicProfile(long _id, ptime _publishDate, std::string _topics)
{
    return buildTopicProfile(_id, _publishDate, _topics);
}

}
