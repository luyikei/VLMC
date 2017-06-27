#ifndef IINFO_H
#define IINFO_H

#include <vector>

#include "IParameterInfo.h"

namespace Backend
{
class IInfo
{
public:
    virtual ~IInfo() = default;

    virtual const std::string&  identifier() const = 0;
    virtual const std::string&  name() const = 0;
    virtual const std::string&  description() const = 0;
    virtual const std::string&  author() const = 0;
    virtual const std::vector<IParameterInfo*>& paramInfos() const = 0;
};
}

#endif // IINFO_H
