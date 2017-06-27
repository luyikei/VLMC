#ifndef IPARAMETERINFO_H
#define IPARAMETERINFO_H

#include <string>

namespace Backend
{
    class IParameterInfo
    {
    public:
        virtual ~IParameterInfo() = default;

        virtual const std::string&  identifier() const = 0;
        virtual const std::string&  name() const = 0;
        virtual const std::string&  type() const = 0;
        virtual const std::string&  description() const = 0;

        virtual const std::string&   defaultValue() const = 0;
        virtual const std::string&   minValue() const = 0;
        virtual const std::string&   maxValue() const = 0;
    };
}

#endif // IPARAMETERINFO_H
