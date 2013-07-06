#ifndef TWEETPROFILE_H
#define TWEETPROFILE_H

#include "profile.h"
#include <vector>
#include <ctime>
#include <QString>
#include <QDateTime>

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
    TweetProfile(long _id, QDateTime _publishDate, ConceptMapPtr _profile);
    long m_id;
    QDateTime m_publishDateTime;
    ConceptMapPtr m_profile;

public:
    virtual ConceptMapPtr getProfile() { return m_profile; }
    virtual QDateTime getPublishDateTime() { return m_publishDateTime; }
    virtual long getTweetId() { return m_id; }

    static TweetProfilePtr getBagOfWordsProfile(long _id, QDateTime _publishDate, QString _tweetContent);
    static TweetProfilePtr getHashtagProfile(long _id, QDateTime _publishDate, QString _tweetContent);
};

}
#endif // TWEETPROFILE_H
