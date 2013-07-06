cmake_minimum_required(VERSION 2.8)

project(gaomodel)

find_package(Qt4 REQUIRED)
set(QT_USE_QTSQL TRUE)
include(${QT_USE_FILE})

add_definitions(-std=c++11 -std=gnu++11 -pthread -W -Wall -Wno-unused-function)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_VERBOSE_MAKEFILE ON)
else()
    add_definitions(-Ofast)
endif()



add_executable(gaomodel evaluation.cpp utils.h profile.h tweetprofile.cpp userprofile.cpp main.cpp)

add_custom_command(TARGET gaomodel COMMAND cp ${CMAKE_SOURCE_DIR}/users_good ${CMAKE_BINARY_DIR})

target_link_libraries(gaomodel ${QT_LIBRARIES})

add_subdirectory(test)
