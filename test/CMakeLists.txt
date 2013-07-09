find_library(LIBPTHREAD NAMES pthread)
find_library(LIBGTEST NAMES gtest)
find_library(LIBGTEST_MAIN NAMES gtest_main)

include_directories(${SQLITE3_INCLUDE_DIRS} ../)

add_executable(tests userprofiletests.cpp ../userprofile.cpp ../tweetprofile.cpp ../dateutils.cpp ../evaluation.cpp)

enable_testing()

target_link_libraries(tests ${LIBGTEST} ${LIBGTEST_MAIN} ${LIBPTHREAD} ${Boost_LIBRARIES} pq pqxx)

add_test(
    NAME tests
    COMMAND tests
)
