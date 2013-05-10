#ifndef UTILS_H
#define UTILS_H
#include <list>

namespace casimiro {

typedef std::list<int> IntegerList;
typedef std::shared_ptr<IntegerList> IntegerListPtr;

typedef std::list<std::string> StringList;
typedef std::shared_ptr<StringList> StringListPtr;

}
#endif // UTILS_H
