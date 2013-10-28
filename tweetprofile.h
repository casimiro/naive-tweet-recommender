#ifndef TWEETPROFILE_H
#define TWEETPROFILE_H

#include <boost/date_time/posix_time/posix_time.hpp>
#include "profile.h"
#include <vector>
#include <string>

namespace casimiro {

using namespace boost::posix_time;

class TweetProfile;
typedef std::shared_ptr<TweetProfile> TweetProfilePtr;
typedef std::vector<TweetProfilePtr> TweetProfileVector;
typedef std::shared_ptr<TweetProfileVector> TweetProfileVectorPtr;

class TweetProfile : public Profile
{
public:
    virtual ~TweetProfile();

private:
    TweetProfile(long _id, ptime _publishDate, ConceptMapPtr _profile);
    long m_id;
    ptime m_publishDateTime;
    ConceptMapPtr m_profile;

    static TweetProfilePtr buildProfile(long _id, ptime _publishDate, std::string _tweetContent, std::string _pattern);
    static TweetProfilePtr buildTopicProfile(long int _id, ptime _publishDate, std::string _topics);

public:
    virtual ConceptMapPtr getProfile() { return m_profile; }
    virtual ptime getPublishDateTime() { return m_publishDateTime; }
    virtual long getTweetId() { return m_id; }

    static TweetProfilePtr getBagOfWordsProfile(long _id, ptime _publishDate, std::string _tweetContent);
    static TweetProfilePtr getHashtagProfile(long _id, ptime _publishDate, std::string _tweetContent);
    static TweetProfilePtr getTopicProfile(long _id, ptime _publishDate, std::string _topics);

    bool operator < (const TweetProfile& _tweetProfile) const
    {
        return (m_publishDateTime < _tweetProfile.m_publishDateTime);
    }
};

}
#endif // TWEETPROFILE_H
