#ifndef USERPROFILE_H
#define USERPROFILE_H
#include <ctime>
#include "profile.h"
#include "newsprofile.h"
#include "utils.h"
#include "sharednews.h"


namespace casimiro {

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
    
    virtual double cosineSimilarity(ConceptMapPtr _profile);
public:
    virtual IntegerListPtr getSortedRecommendations(NewsProfileListPtr _newsProfiles, std::tm _until);
    virtual StringListPtr getSharedNewsLinks();
    virtual SharedNewsVectorPtr getSharedNews(std::tm _start, std::tm _end);
    virtual ConceptMapPtr getProfile() { return m_profile; }
    virtual long getUserId() { return m_userId; }
    virtual std::tm getStart() { return m_start; }
    virtual std::tm getEnd() { return m_end; }
    
    static UserProfilePtr getUserProfile(long _userId, std::tm _start, std::tm _end);
};

}
#endif // USERPROFILE_H
