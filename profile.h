#ifndef PROFILE_H
#define PROFILE_H

#include <memory>
#include <string>
#include <map>

namespace casimiro {

typedef std::map<std::string, float> ConceptMap;
typedef std::shared_ptr<ConceptMap> ConceptMapPtr;

class Profile
{
public:
    virtual ConceptMapPtr getProfile() = 0;
};

}
#endif // PROFILE_H
