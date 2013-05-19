cmake_minimum_required(VERSION 2.8)

project(gaomodel)

find_package(Qt4 REQUIRED)
set(QT_USE_QTSQL TRUE)
include(${QT_USE_FILE})

add_definitions(-std=c++11 -std=gnu++11 -pthread -W -Wall -Wno-unused-function -Ofast)
add_executable(gaomodel utils.h profile.h tweetprofile.cpp userprofile.cpp main.cpp)

add_custom_command(TARGET gaomodel COMMAND cp ${CMAKE_SOURCE_DIR}/wi2011-user-sample.csv ${CMAKE_BINARY_DIR})

target_link_libraries(gaomodel ${QT_LIBRARIES})

add_subdirectory(test)
