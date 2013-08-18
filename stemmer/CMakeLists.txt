find_library(LIBPTHREAD NAMES pthread)

include_directories(${libpq_INCLUDE_DIRS} ${libpqxx_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
add_executable(indexer indexer.cpp rslpStemmer.c rslpTrie.c)
target_link_libraries(indexer ${Boost_LIBRARIES} pq pqxx icui18n icuuc icudata)
