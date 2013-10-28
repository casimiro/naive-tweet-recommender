#include <ctime>
#include <iostream>
#include <fstream>
#include <pqxx/pqxx>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "userprofile.h"
#include "evaluation.h"
#include "dateutils.h"

using namespace casimiro;
using namespace boost::posix_time;

int main(int /*argc*/, char** /*argv*/) {

    ptime startTraining = time_from_string("2013-01-01 00:00:00");
    ptime endTraining = time_from_string("2013-05-01 00:00:00");
    ptime startTest = time_from_string("2013-05-01 00:00:01");
    ptime endTest = time_from_string("2013-08-01 00:00:00");
    
    LongVector userIds;

    std::ifstream users("users_good");
    std::string line;
    while(std::getline(users, line))
        userIds.push_back(atol(line.c_str()));

    Evaluation evaluation("postgresql://tweets:zxc123@192.168.25.33:5432/tweets", 
                          std::make_shared<LongVector>(userIds), 
                          startTraining, 
                          endTraining, 
                          startTest, 
                          endTest, 
                          RANDOM_EVAL,
                          false);

    evaluation.run();
}
