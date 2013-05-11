#ifndef SHAREDNEWS_H
#define SHAREDNEWS_H
#include <ctime>
#include <vector>
#include <memory>

namespace casimiro {

class SharedNews;
typedef std::shared_ptr<SharedNews> SharedNewsPtr;
typedef std::vector<SharedNews> SharedNewsVector;
typedef std::shared_ptr<SharedNewsVector> SharedNewsVectorPtr;

class SharedNews
{
public:
    SharedNews(int _newsId, std::tm _sharedAt);
    virtual ~SharedNews();

private:
    int m_newsId;
    std::tm m_sharedAt;
    std::time_t m_sharedAtTime;

public:
    virtual int getNewsId() { return m_newsId; }
    virtual std::tm getSharedAt() { return m_sharedAt; }
    virtual std::time_t getSharedAtTime() { return m_sharedAtTime; }
};

}
#endif // SHAREDNEWS_H
