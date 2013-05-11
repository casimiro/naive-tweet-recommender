#include "sharednews.h"
#include <exception>

namespace casimiro {

SharedNews::SharedNews(int _newsId, std::tm _sharedAt):
    m_newsId(_newsId),
    m_sharedAt(_sharedAt)
{
    m_sharedAtTime = mktime(&_sharedAt);
    if(m_sharedAtTime < 0)
        throw std::exception();
}

SharedNews::~SharedNews()
{
}

}
