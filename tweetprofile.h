#ifndef TWEETPROFILE_H
#define TWEETPROFILE_H

#include "profile.h"
#include <vector>
#include <ctime>
#include <string>
#include <ctime>

namespace casimiro {

class TweetProfile;
typedef std::shared_ptr<TweetProfile> TweetProfilePtr;
typedef std::vector<TweetProfilePtr> TweetProfileVector;
typedef std::shared_ptr<TweetProfileVector> TweetProfileVectorPtr;

class TweetProfile : public Profile
{
public:
    virtual ~TweetProfile();

private:
    TweetProfile(long _id, std::tm _publishDate, ConceptMapPtr _profile);
    long m_id;
    std::tm m_publishDateTime;
    ConceptMapPtr m_profile;

    static TweetProfilePtr buildProfile(long _id, std::tm _publishDate, std::string _tweetContent, std::string _pattern);

public:
    virtual ConceptMapPtr getProfile() { return m_profile; }
    virtual std::tm getPublishDateTime() { return m_publishDateTime; }
    virtual long getTweetId() { return m_id; }

    static TweetProfilePtr getBagOfWordsProfile(long _id, std::tm _publishDate, std::string _tweetContent);
    static TweetProfilePtr getHashtagProfile(long _id, std::tm _publishDate, std::string _tweetContent);
};

}
#endif // TWEETPROFILE_H
