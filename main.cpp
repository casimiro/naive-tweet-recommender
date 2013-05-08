#include <ctime>
#include <iostream>
#include <fstream>

#include <QtSql/QtSql>

#include "profiles.h"

using namespace casimiro;

int main(int /*argc*/, char** /*argv*/) {
    
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("localhost");
    db.setDatabaseName("tweets");
    db.setUserName("root");
    db.setPassword("zxc123");
    
    if(!db.open())
    {
        std::cerr << "Could not open database" << std::endl;
        return -1;
    }
    
    std::tm start;
    std::tm end;
    
    start.tm_year = 2000 - 1900;
    start.tm_mon = 11;
    start.tm_mday = 20;
    start.tm_hour = 0;
    start.tm_min = 0;
    
    end.tm_year = 2010 - 1900;
    end.tm_mon = 11;
    end.tm_mday = 19;
    end.tm_hour = 0;
    end.tm_min = 0;
    
    std::tm startEvaluation;
    std::tm endEvaluation;
    
    startEvaluation.tm_year = 2010 - 1900;
    startEvaluation.tm_mon = 11;
    startEvaluation.tm_mday = 20;
    startEvaluation.tm_hour = 0;
    startEvaluation.tm_min = 0;
    
    endEvaluation.tm_year = 2010 - 1900;
    endEvaluation.tm_mon = 11;
    endEvaluation.tm_mday = 26;
    endEvaluation.tm_hour = 23;
    endEvaluation.tm_min = 59;

    NewsProfileListPtr newsProfiles = NewsProfile::getNewsProfilesBetween(startEvaluation, endEvaluation);
    
    std::ifstream file("users");
    
    std::string line;
    double meanPosition;
    while(std::getline(file, line))
    {
        long userId = atol(line.c_str());
        UserProfilePtr uProfile = UserProfile::getUserProfile(userId, start, end);
        IntegerListPtr recommendedNews = uProfile->getSortedRecommendations(newsProfiles);
        IntegerListPtr newsShared = uProfile->getSharedNews(startEvaluation, endEvaluation);
        
        meanPosition = 0;
        
    }
    
    file.close();
}
