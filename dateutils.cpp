#include "dateutils.h"

DateUtils::DateUtils()
{
}

std::string DateUtils::TmToString(std::tm _tm)
{
    char s[30];
    strftime(s, 30, "%Y-%m-%d %H:%M:%S", &_tm);
    return std::string(s);
}

std::tm DateUtils::StringToTm(std::string _date)
{
    std::tm tmDate;
    strptime(_date.c_str(), "%Y-%m-%d %H:%M:%S", &tmDate);
    return tmDate;
}
