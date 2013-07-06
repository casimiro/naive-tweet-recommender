#include <ctime>
#include <iostream>
#include <fstream>

#include <QtSql/QtSql>

#include "userprofile.h"
#include "evaluation.h"

using namespace casimiro;

int main(int /*argc*/, char** /*argv*/) {
    
    QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");
    db.setHostName("localhost");
    db.setDatabaseName("tweetsbr2");
    db.setUserName("tweetsbr");
    db.setPassword("zxc123");

    if(!db.open())
    {
        std::cerr << "Could not open database" << std::endl;
        return -1;
    }
    
    QDateTime startTraining = QDateTime::fromString("2001-01-01 00:00:00", "yyyy-MM-dd HH:mm:ss");
    QDateTime endTraining = QDateTime::fromString("2013-04-01 00:00:00", "yyyy-MM-dd HH:mm:ss");
    QDateTime startTest = QDateTime::fromString("2013-04-01 00:00:01", "yyyy-MM-dd HH:mm:ss");
    QDateTime endTest = QDateTime::fromString("2013-05-01 00:00:00", "yyyy-MM-dd HH:mm:ss");
    LongVector userIds;

    std::ifstream users("users_good");
    std::string line;
    while(std::getline(users, line))
        userIds.push_back(atol(line.c_str()));

    Evaluation evaluation(std::make_shared<LongVector>(userIds), startTraining, endTraining, startTest, endTest);

    evaluation.run();
}
