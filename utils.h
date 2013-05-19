#ifndef UTILS_H
#define UTILS_H
#include <list>
#include <vector>
#include <memory>

namespace casimiro {

typedef std::list<int> IntegerList;
typedef std::shared_ptr<IntegerList> IntegerListPtr;

typedef std::vector<long> LongVector;
typedef std::shared_ptr<LongVector> LongVectorPtr;

typedef std::list<std::string> StringList;
typedef std::shared_ptr<StringList> StringListPtr;

}
#endif // UTILS_H
