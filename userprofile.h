#ifndef USERPROFILE_H
#define USERPROFILE_H
#include <ctime>
#include "profile.h"
#include "utils.h"
#include "tweetprofile.h"
#include <QDateTime>

namespace casimiro {

enum ProfileType {
    HASHTAG,
    BAG_OF_WORDS
};

class UserProfile;
typedef std::shared_ptr<UserProfile> UserProfilePtr;

typedef std::vector<std::pair<QDateTime, long>> RetweetVector;
typedef std::shared_ptr<RetweetVector> RetweetVectorPtr;

class UserProfile : public Profile
{
public:
    UserProfile(long _userId, ConceptMapPtr _profile, QDateTime _start, QDateTime _end, ProfileType _profileType);
    virtual ~UserProfile();
    
private:
    long m_userId;
    ConceptMapPtr m_profile;
    QDateTime m_start;
    QDateTime m_end;
    ProfileType m_profileType;
    
    virtual double cosineSimilarity(ConceptMapPtr _profile);
public:
    virtual TweetProfileVectorPtr getCandidateTweets(QDateTime _start, QDateTime _end);
    virtual RetweetVectorPtr getRetweets(QDateTime _start, QDateTime _end);
    virtual ConceptMapPtr getProfile() { return m_profile; }
    virtual long getUserId() { return m_userId; }
    virtual QDateTime getStart() { return m_start; }
    virtual QDateTime getEnd() { return m_end; }
    
    static UserProfilePtr getHashtagProfile(long _userId, QDateTime _start, QDateTime _end, bool _social);
    static UserProfilePtr getBagOfWordsProfile(long _userId, QDateTime _start, QDateTime _end, bool _social);
};

}
#endif // USERPROFILE_H
