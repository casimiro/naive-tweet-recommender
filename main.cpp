#include <ctime>
#include <iostream>
#include <fstream>

#include <QtSql/QtSql>

#include "userprofile.h"
#include "newsprofile.h"
#include "sharednews.h"

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
    strptime("2000-12-20 00:00:00", "%Y-%m-%d %H:%H:%S", &start);
    std::tm end;
    strptime("2010-12-19 00:00:00", "%Y-%m-%d %H:%H:%S", &start);

    std::tm startEvaluation;
    strptime("2010-12-20 00:00:00", "%Y-%m-%d %H:%H:%S", &start);

    std::tm startNewsEvaluation;
    strptime("2010-12-13 00:00:00", "%Y-%m-%d %H:%H:%S", &start);

    std::tm endEvaluation;
    strptime("2010-12-26 23:59:59", "%Y-%m-%d %H:%H:%S", &start);
    
    NewsProfileListPtr newsProfiles = NewsProfile::getNewsProfilesBetween(startNewsEvaluation, endEvaluation);
    
    std::ifstream file("users");
    
    std::string line;
    double meanPosition;
    while(std::getline(file, line))
    {
        long userId = atol(line.c_str());
        UserProfilePtr uProfile = UserProfile::getUserProfile(userId, start, end);
        SharedNewsVectorPtr newsShared = uProfile->getSharedNews(startEvaluation, endEvaluation);
        
        auto newsIt = newsShared->begin();
        auto newsEnd = newsShared->end();

        if(newsIt == newsEnd)
            continue;

        for (; newsIt != newsEnd; newsIt++)
        {
            auto recs = uProfile->getSortedRecommendations(newsProfiles, newsIt->getSharedAt());

        }

        meanPosition = 0;
        
    }
    
    file.close();
}
