#ifndef USERPROFILE_H
#define USERPROFILE_H
#include <boost/date_time/posix_time/posix_time.hpp>
#include <pqxx/pqxx>

#include "profile.h"
#include "utils.h"
#include "tweetprofile.h"


namespace casimiro {

using namespace boost::posix_time;

enum ProfileType {
    HASHTAG,
    BAG_OF_WORDS,
    TOPICS
};

class UserProfile;
typedef std::shared_ptr<UserProfile> UserProfilePtr;

typedef std::vector<std::pair<ptime, long>> RetweetVector;
typedef std::shared_ptr<RetweetVector> RetweetVectorPtr;

class UserProfile : public Profile
{
public:
    UserProfile(PqConnectionPtr _con, long _userId, ConceptMapPtr _profile, ptime _start, ptime _end, ProfileType _profileType, std::string _sqlQuery);
    virtual ~UserProfile();
    
private:
    PqConnectionPtr m_con;
    long m_userId;
    ConceptMapPtr m_profile;
    ptime m_start;
    ptime m_end;
    ProfileType m_profileType;
    std::string m_sqlQuery;
    
    virtual void buildConceptMap(pqxx::result &_rows, std::string _pattern);

public:
    virtual TweetProfileVectorPtr getCandidateTweets(const ptime &_start, const ptime &_end);
    virtual RetweetVectorPtr getRetweets(const ptime &_start, const ptime &_end);
    virtual void loadProfile();
    
    virtual double cosineSimilarity(ConceptMapPtr _profile);

    virtual ConceptMapPtr getProfile() { return m_profile; }
    virtual long getUserId() { return m_userId; }
    virtual ptime getStart() { return m_start; }
    virtual ptime getEnd() { return m_end; }

    
    static UserProfilePtr getHashtagProfile(PqConnectionPtr _con, long _userId, ptime _start, ptime _end, bool _social);
    static UserProfilePtr getBagOfWordsProfile(PqConnectionPtr _con, long _userId, ptime _start, ptime _end, bool _social);
};

}
#endif // USERPROFILE_H
