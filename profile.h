#ifndef PROFILE_H
#define PROFILE_H

#include <memory>
#include <string>
#include <map>

namespace casimiro {

class EmptyProfileException : public std::exception
{
public:
    EmptyProfileException() throw() {}
    virtual ~EmptyProfileException() throw() {}

public:
    virtual const char * what() const throw()
    {
        return "Empty profile loaded.";
    }
};

typedef std::map<std::string, float> ConceptMap;
typedef std::shared_ptr<ConceptMap> ConceptMapPtr;

class Profile
{
public:
    virtual ConceptMapPtr getProfile() = 0;
};

}
#endif // PROFILE_H
