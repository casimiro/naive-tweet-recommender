#ifndef USERPROFILE_H
#define USERPROFILE_H
#include <map>
#include <list>
#include <ctime>
#include <string>
#include <memory>

namespace casimiro {
    
// Typedefs
typedef std::map<std::string, float> ConceptMap;
typedef std::shared_ptr<ConceptMap> ConceptMapPtr;

typedef std::list<int> IntegerList;
typedef std::shared_ptr<IntegerList> IntegerListPtr;

typedef std::list<std::string> StringList;
typedef std::shared_ptr<StringList> StringListPtr;

class UserProfile;
typedef std::shared_ptr<UserProfile> UserProfilePtr;

class NewsProfile;
typedef std::shared_ptr<NewsProfile> NewsProfilePtr;
typedef std::shared_ptr<std::list<NewsProfilePtr>> NewsProfileListPtr;

class Profile
{
public:
    virtual ConceptMapPtr getProfile() = 0;
};

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
    virtual IntegerListPtr getSortedRecommendations(NewsProfileListPtr _newsProfiles);
    virtual StringListPtr getSharedNewsLinks();
    virtual IntegerListPtr getSharedNews(std::tm _start, std::tm _end);
    virtual ConceptMapPtr getProfile() { return m_profile; }
    virtual long getUserId() { return m_userId; }
    virtual std::tm getStart() { return m_start; }
    virtual std::tm getEnd() { return m_end; }
    
    static UserProfilePtr getUserProfile(long _userId, std::tm _start, std::tm _end);
    
};

class NewsProfile : public Profile 
{
public:
    NewsProfile(int _newsId, ConceptMapPtr _profile);
    virtual ~NewsProfile();

private:
    int m_newsId;
    ConceptMapPtr m_profile;

public:
    virtual ConceptMapPtr getProfile() { return m_profile; }
    virtual int getNewsId() { return m_newsId; }
    static NewsProfilePtr getNewsProfile(int _newsId);
    static NewsProfileListPtr getNewsProfilesBetween(std::tm _start, std::tm _end);
    
};


}
#endif // USERPROFILE_H
