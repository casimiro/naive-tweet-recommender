#ifndef NEWS_H
#define NEWS_H

class News
{
public:
    News(int _id, std::tm _publishedDate);
    virtual ~News();

private:
    int m_id;
    std::tm m_publishedDate;
};

#endif // NEWS_H
