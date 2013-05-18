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
    strptime("2010-12-19 00:00:00", "%Y-%m-%d %H:%H:%S", &end);

    std::tm startEvaluation;
    strptime("2010-12-20 00:00:00", "%Y-%m-%d %H:%H:%S", &startEvaluation);

    std::tm endEvaluation;
    strptime("2010-12-26 23:59:59", "%Y-%m-%d %H:%H:%S", &endEvaluation);
    
    NewsProfileListPtr newsProfiles = NewsProfile::getNewsProfilesBetween(startEvaluation, endEvaluation);
    
    std::ifstream file("wi2011-user-sample.csv");
    
    std::string line;

    int users = 0;
    double globalMeanPosition = 0;
    double userMeanPosition;
    int newsFound;
    while(std::getline(file, line))
    {
        long userId = atol(line.c_str());
        UserProfilePtr uProfile = UserProfile::getUserProfile(userId, start, end);
        SharedNewsVectorPtr newsShared = uProfile->getSharedNews(startEvaluation, endEvaluation);
        auto recs = uProfile->getSortedRecommendations(newsProfiles, endEvaluation);
        newsFound = 0;
        userMeanPosition = 0;
        for (auto newsIt = newsShared->begin(); newsIt != newsShared->end(); newsIt++)
        {

            auto found = std::find(recs->begin(), recs->end(), newsIt->getNewsId());

            if(found != recs->end())
            {
                newsFound++;
                double pos = (double)std::distance(recs->begin(), found);
                userMeanPosition += 1/(pos+1);
            }

        }

        userMeanPosition = userMeanPosition / newsShared->size();
        std::cout << "User mean pos: " << userMeanPosition << std::endl;
        globalMeanPosition += userMeanPosition;
        users++;
    }

    globalMeanPosition = globalMeanPosition / users;
    std::cout << "Global mean pos: " << globalMeanPosition << std::endl;
    std::cout << "Users: " << users << std::endl;
    file.close();
}
