#include <ctime>
#include <iostream>
#include <fstream>

#include <QtSql/QtSql>

#include "userprofile.h"

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

}
