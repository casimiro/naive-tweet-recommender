#include "tweetprofile.h"
#include <QRegExp>

namespace casimiro {

TweetProfile::~TweetProfile()
{
}

TweetProfile::TweetProfile(int _id, QDateTime _publishDate, ConceptMapPtr _profile):
    m_id(_id),
    m_publishDate(_publishDate),
    m_profile(_profile)
{
}

TweetProfilePtr TweetProfile::getHashtagProfile(int _id, QDateTime _publishDate, QString _tweetContent)
{
    ConceptMap conceptMap;
    QRegExp rx("#\\w+");
    int pos = 0;
    float sum = 0;
    while ((pos = rx.indexIn(_tweetContent, pos)) != -1)
    {
        std::string match = rx.cap(0).toLower().toStdString();
        if(conceptMap.find(match) == conceptMap.end())
            conceptMap.insert(std::make_pair(match, 0));

        conceptMap[match] += 1;
        sum++;
        pos += rx.matchedLength();
    }
    return TweetProfilePtr(new TweetProfile(_id, _publishDate, std::make_shared<ConceptMap>(conceptMap)));
}

}
