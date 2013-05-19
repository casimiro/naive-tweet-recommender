#ifndef TWEETPROFILE_H
#define TWEETPROFILE_H

#include "profile.h"
#include <vector>
#include <ctime>
#include <QString>

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
    TweetProfile(int _id, std::tm _publishDate, ConceptMapPtr _profile);
    int m_id;
    std::tm m_publishDate;
    ConceptMapPtr m_profile;

public:
    static TweetProfilePtr getHashtagProfile(int _id, std::tm _publishDate, QString _tweetContent);
    virtual ConceptMapPtr getProfile() { return m_profile; }
};

}
#endif // TWEETPROFILE_H
