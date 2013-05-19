#ifndef USERPROFILE_H
#define USERPROFILE_H
#include <ctime>
#include "profile.h"
#include "utils.h"
#include "tweetprofile.h"

namespace casimiro {

enum ProfileType {
    HASHTAG
};

class UserProfile;
typedef std::shared_ptr<UserProfile> UserProfilePtr;

class UserProfile : public Profile
{
public:
    UserProfile(long _userId, ConceptMapPtr _profile, std::tm _start, std::tm _end);
    virtual ~UserProfile();
    
private:
    long m_userId;
    ConceptMapPtr m_profile;
    std::tm m_start;
    std::tm m_end;
    ProfileType m_profileType;
    
    virtual double cosineSimilarity(ConceptMapPtr _profile);
public:
    virtual TweetProfileVectorPtr getCandidateTweets(std::tm _start, std::tm _end);
    virtual ConceptMapPtr getProfile() { return m_profile; }
    virtual long getUserId() { return m_userId; }
    virtual std::tm getStart() { return m_start; }
    virtual std::tm getEnd() { return m_end; }
    
    static UserProfilePtr getHashtagProfile(long _userId, std::tm _start, std::tm _end);
};

}
#endif // USERPROFILE_H
