find_library(LIBPTHREAD NAMES pthread)
find_library(LIBGTEST NAMES gtest)
find_library(LIBGTEST_MAIN NAMES gtest_main)

add_executable(tests userprofiletests.cpp ../profiles.cpp)

enable_testing()

target_link_libraries(tests ${LIBGTEST} ${LIBGTEST_MAIN} ${LIBPTHREAD} ${QT_LIBRARIES})

add_test(
    NAME tests
    COMMAND tests
)