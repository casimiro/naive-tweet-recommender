#ifndef DATEUTILS_H
#define DATEUTILS_H
#include <string>
#include <ctime>

class DateUtils
{
public:
    DateUtils();

    static std::string TmToString(std::tm _tm);
    static std::tm StringToTm(std::string _date);
};

#endif // DATEUTILS_H
