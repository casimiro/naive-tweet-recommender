cmake_minimum_required(VERSION 2.8)

project(gaomodel)

find_library(pq REQUIRED)
find_library(pqxx REQUIRED)
find_package(Boost COMPONENTS regex date_time REQUIRED)

include_directories(${libpq_INCLUDE_DIRS} ${libpqxx_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})

add_definitions(-std=c++11 -std=gnu++11 -pthread -W -Wall -Wno-unused-function)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_VERBOSE_MAKEFILE ON)
else()
    add_definitions(-O3)
endif()



add_executable(gaomodel evaluation.cpp utils.h profile.h tweetprofile.cpp userprofile.cpp dateutils.cpp main.cpp)


add_custom_command(TARGET gaomodel COMMAND cp ${CMAKE_SOURCE_DIR}/users_good ${CMAKE_BINARY_DIR})

target_link_libraries(gaomodel ${Boost_LIBRARIES} pq pqxx)

add_subdirectory(test)
add_subdirectory(stemmer)
