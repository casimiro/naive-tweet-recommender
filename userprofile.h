#ifndef USERPROFILE_H
#define USERPROFILE_H
#include <ctime>
#include <pqxx/pqxx>

#include "profile.h"
#include "utils.h"
#include "tweetprofile.h"


namespace casimiro {

enum ProfileType {
    HASHTAG,
    BAG_OF_WORDS
};

class UserProfile;
typedef std::shared_ptr<UserProfile> UserProfilePtr;

typedef std::vector<std::pair<std::tm, long>> RetweetVector;
typedef std::shared_ptr<RetweetVector> RetweetVectorPtr;

class UserProfile : public Profile
{
public:
    UserProfile(PqConnectionPtr _con, long _userId, ConceptMapPtr _profile, std::tm _start, std::tm _end, ProfileType _profileType, std::string _sqlQuery);
    virtual ~UserProfile();
    
private:
    PqConnectionPtr m_con;
    long m_userId;
    ConceptMapPtr m_profile;
    std::tm m_start;
    std::tm m_end;
    ProfileType m_profileType;
    std::string m_sqlQuery;
    
    virtual double cosineSimilarity(ConceptMapPtr _profile);
    virtual void buildConceptMap(pqxx::result &_rows, std::string _pattern);
public:
    virtual TweetProfileVectorPtr getCandidateTweets(std::tm _start, std::tm _end);
    virtual RetweetVectorPtr getRetweets(std::tm _start, std::tm _end);
    virtual void loadProfile();

    virtual ConceptMapPtr getProfile() { return m_profile; }
    virtual long getUserId() { return m_userId; }
    virtual std::tm getStart() { return m_start; }
    virtual std::tm getEnd() { return m_end; }

    
    static UserProfilePtr getHashtagProfile(PqConnectionPtr _con, long _userId, std::tm _start, std::tm _end, bool _social);
    static UserProfilePtr getBagOfWordsProfile(PqConnectionPtr _con, long _userId, std::tm _start, std::tm _end, bool _social);
};

}
#endif // USERPROFILE_H
