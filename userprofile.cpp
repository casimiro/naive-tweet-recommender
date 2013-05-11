#include "userprofile.h"
#include <QtSql/QtSql>
#include <iostream>
#include <vector>

namespace casimiro {
    
UserProfile::UserProfile(long _userId, ConceptMapPtr _profile, std::tm _start, std::tm _end):
    m_userId(_userId),
    m_profile(_profile),
    m_start(_start),
    m_end(_end)
{
}

UserProfile::~UserProfile()
{
}

UserProfilePtr UserProfile::getUserProfile(long _userId, std::tm _start, std::tm _end)
{
    ConceptMapPtr mPtr = std::make_shared<ConceptMap>();
    UserProfilePtr up = std::make_shared<UserProfile>(_userId, mPtr, _start, _end);
    
    char s[19];
    char e[19];
    s[18] = '\0';
    s[18] = '\0';
    strftime(s, 19, "%Y-%m-%d %H:%M", &_start);
    strftime(e, 19, "%Y-%m-%d %H:%M", &_end);
    
    QSqlQuery query;
    query.prepare("SELECT uri,count(uri) FROM tweets_sample as t, semanticsTweetsEntity as s WHERE t.userId = :uid AND t.creationTime >= :start AND t.creationTime <= :end AND t.id = s.tweetId GROUP BY uri");
    query.bindValue(":uid", (qlonglong) _userId);
    query.bindValue(":start", s);
    query.bindValue(":end", e);
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
    
    return up;
}

double UserProfile::cosineSimilarity(ConceptMapPtr _profile)
{
    float aNorm,bNorm,dot;
    aNorm = bNorm = dot = 0;
    
    auto uIt = m_profile->begin();
    auto nIt = _profile->begin();
    
    auto uEnd = m_profile->end();
    auto nEnd = _profile->end();
    
    int compare;
    while (uIt != uEnd && nIt != nEnd)
    {
        compare = uIt->first.compare(nIt->first);
        if(compare == 0)
        {
            dot += uIt->second * nIt->second;
            uIt++;
            nIt++;
        }
        else if(compare < 0)
            uIt++;
        else if(compare > 0)
            nIt++;
    }
    
    for (uIt = m_profile->begin(); uIt != uEnd; uIt++)
        aNorm += pow(uIt->second, 2);
    aNorm = sqrt(aNorm);
    
    for (nIt = _profile->begin(); nIt != nEnd; nIt++)
        bNorm += pow(nIt->second, 2);
    bNorm = sqrt(bNorm);
    
    return dot / (aNorm * bNorm);
}

IntegerListPtr UserProfile::getSortedRecommendations(NewsProfileListPtr _newsProfiles, std::tm _until)
{
    std::time_t untilTime = mktime(&_until);
    std::map<double, std::vector<int>> aux;
    auto end = aux.end();
    
    double sim;
    for (auto it = _newsProfiles->begin(); it != _newsProfiles->end(); it++)
    {
        if(it->get()->getPublishTime() > untilTime)
            continue;

        sim = cosineSimilarity(it->get()->getProfile());
        if(aux.find(sim) == end)
            aux[sim] = std::vector<int>();
        aux[sim].push_back(it->get()->getNewsId());
    }
    
    IntegerListPtr sortedNews = std::make_shared<IntegerList>();
    for (auto it = aux.begin(); it != end; it++)
    {
        for (auto newsId : it->second)
        {
            sortedNews->push_back(newsId);
        }
    }
    return sortedNews;
}

SharedNewsVectorPtr UserProfile::getSharedNews(std::tm _start, std::tm _end)
{
    SharedNewsVectorPtr news = std::make_shared<SharedNewsVector>();
    char s[19];
    char e[19];
    s[18] = '\0';
    s[18] = '\0';
    strftime(s, 19, "%Y-%m-%d %H:%M", &_start);
    strftime(e, 19, "%Y-%m-%d %H:%M", &_end);
    
    QSqlQuery query;
    query.prepare("SELECT newsId, creationTime FROM nas WHERE userId = :uid AND creationTime >= :start AND creationTime <= :end AND (strategy=31211 OR strategy=31111 OR strategy=13261)");
    query.bindValue(":uid", (qlonglong) m_userId);
    query.bindValue(":start", s);
    query.bindValue(":end", e);
    query.exec();
    
    while(query.next())
    {
        std::string dateString = query.value(1).toString().toStdString();
        std::tm date;
        strptime(dateString.c_str(), "%Y-%m-%d %H:%M:%S", &date);

        news->push_back(SharedNews(query.value(0).toInt(), date));
    }
    
    return news;
}

StringListPtr UserProfile::getSharedNewsLinks()
{
    StringListPtr news = std::make_shared<StringList>();
    char s[19];
    char e[19];
    s[18] = '\0';
    s[18] = '\0';
    strftime(s, 19, "%Y-%m-%d %H:%M", &m_start);
    strftime(e, 19, "%Y-%m-%d %H:%M", &m_end);
    
    QSqlQuery query;
    query.prepare("SELECT content FROM tweets_sample as t WHERE t.userId = :uid AND t.creationTime >= :start AND t.creationTime <= :end AND content LIKE '%http%'");
    query.bindValue(":uid", (qlonglong) m_userId);
    query.bindValue(":start", s);
    query.bindValue(":end", e);
    query.exec();
    
    QRegExp rx("(http://(?:bbc\\.in|cnn|nyti\\.ms|on\\.cnn)[^\\(\\)\"' @#]+)");
    while(query.next())
    {
        QString content = query.value(0).toString();
        int pos = 0;
        while ((pos = rx.indexIn(content, pos)) != -1)
        {
            QString match = rx.cap(0);
            QStringList vals = match.split("/");
            if(vals.size() == 4 && vals.at(3).size() > 4)
                news->push_back(match.toStdString());
            else
                std::cout << match.toStdString() << std::endl;
            pos += rx.matchedLength();
        }
    }
    
    return news;
}

}
