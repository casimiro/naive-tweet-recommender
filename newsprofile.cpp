#include "newsprofile.h"
#include <memory>
#include <QtSql/QtSql>

namespace casimiro {

NewsProfile::NewsProfile(int _newsId, std::tm _publishDate, ConceptMapPtr _profile):
    m_newsId(_newsId),
    m_publishDate(_publishDate),
    m_profile(_profile)
{
}

NewsProfile::~NewsProfile()
{
}

NewsProfilePtr NewsProfile::getNewsProfile(int _newsId)
{
    ConceptMapPtr mPtr = std::make_shared<ConceptMap>();
    
    QSqlQuery query;

    // Getting publish date
    query.prepare("SELECT publish_date FROM news as n WHERE n.id = :uid");
    query.bindValue(":uid", _newsId);
    query.exec();

    query.next();
    std::string dateString = query.value(0).toString().toStdString();
    std::tm date;
    strptime(dateString.c_str(), "%Y-%m-%d %H:%M:%S", &date);

    // Getting entities references
    query.prepare("SELECT uri,count(uri) FROM news as n, semanticsNewsEntity as s WHERE n.id = :uid AND n.id = s.newsId GROUP BY uri");
    query.bindValue(":uid", _newsId);
    query.exec();
    
    float sum = 0;
    while(query.next())
    {
        std::string element = query.value(0).toString().toStdString();
        float elementCount = query.value(1).toFloat();
        mPtr->insert(std::make_pair(element, elementCount));
        sum += elementCount;
    }
    
    for(auto it = mPtr->begin(); it != mPtr->end(); it++)
        it->second = it->second / sum;
    
    return std::make_shared<NewsProfile>(_newsId, date, mPtr);
}

NewsProfileListPtr NewsProfile::getNewsProfilesBetween(std::tm _start, std::tm _end)
{
    QSqlQuery query;
    NewsProfileListPtr newsProfileList = NewsProfileListPtr(new std::list<NewsProfilePtr>());
    
    char s[19];
    char e[19];
    s[18] = '\0';
    s[18] = '\0';
    strftime(s, 19, "%Y-%m-%d %H:%M", &_start);
    strftime(e, 19, "%Y-%m-%d %H:%M", &_end);
    
    query.prepare("SELECT id, publish_date FROM news as n WHERE n.publish_date >= :start AND n.publish_date <= :end");
    query.bindValue(":start", s);
    query.bindValue(":end", e);
    
    query.exec();
    
    while(query.next())
    {
        int newsId = query.value(0).toInt();
        NewsProfilePtr newsProfile = NewsProfile::getNewsProfile(newsId);
        newsProfileList->push_back(newsProfile);
    }
    
    return newsProfileList;
}

}
