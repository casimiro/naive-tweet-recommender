#include "news.h"

News::News(int _id, std::tm _publishedDate):
    m_id(_id),
    m_publishedDate(_publishedDate)
{
}

News::~News()
{
}
