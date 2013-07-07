#include <ctime>
#include <iostream>
#include <fstream>
#include <pqxx/pqxx>

#include "userprofile.h"
#include "evaluation.h"
#include "dateutils.h"

using namespace casimiro;

int main(int /*argc*/, char** /*argv*/) {
    auto con = std::make_shared<pqxx::connection>("postgresql://tweetsbr:zxc123@localhost:5432/tweetsbr2");

    if(!con->is_open())
    {
        std::cerr << "Could not open database" << std::endl;
        return -1;
    }

    std::tm startTraining = DateUtils::StringToTm("2001-01-01 00:00:00");
    std::tm endTraining = DateUtils::StringToTm("2013-04-01 00:00:00");
    std::tm startTest = DateUtils::StringToTm("2013-04-01 00:00:01");
    std::tm endTest = DateUtils::StringToTm("2013-05-01 00:00:00");
    
    LongVector userIds;

    std::ifstream users("users_good");
    std::string line;
    while(std::getline(users, line))
        userIds.push_back(atol(line.c_str()));

    Evaluation evaluation(con, std::make_shared<LongVector>(userIds), startTraining, endTraining, startTest, endTest);

    evaluation.run();
}
