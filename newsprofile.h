#ifndef NEWSPROFILE_H
#define NEWSPROFILE_H

#include "profile.h"
#include <memory>
#include <list>
#include <ctime>

namespace casimiro {

class NewsProfile;
typedef std::shared_ptr<NewsProfile> NewsProfilePtr;
typedef std::shared_ptr<std::list<NewsProfilePtr>> NewsProfileListPtr;


class NewsProfile : public Profile 
{
public:
    NewsProfile(int _newsId, std::tm _publishDate, ConceptMapPtr _profile);
    virtual ~NewsProfile();

private:
    int m_newsId;
    std::tm m_publishDate;
    ConceptMapPtr m_profile;
    std::time_t m_publishTime;

public:
    virtual ConceptMapPtr getProfile() { return m_profile; }
    virtual int getNewsId() { return m_newsId; }
    virtual std::time_t getPublishTime() { return m_publishTime; }
    static NewsProfilePtr getNewsProfile(int _newsId);
    static NewsProfileListPtr getNewsProfilesBetween(std::tm _start, std::tm _end);
    
};

}
#endif // NEWSPROFILE_H
