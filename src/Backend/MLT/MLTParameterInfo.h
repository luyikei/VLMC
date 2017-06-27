#ifndef MLTPARAMETERINFO_H
#define MLTPARAMETERINFO_H

#include "Backend/IParameterInfo.h"

namespace Mlt
{
class Properties;
}

namespace Backend
{
namespace MLT
{
    class MLTParameterInfo : public IParameterInfo
    {
    public:
        MLTParameterInfo() = default;
        virtual const std::string&  identifier() const override;
        virtual const std::string&  name() const override;
        virtual const std::string&  type() const override;
        virtual const std::string&  description() const override;

        virtual const std::string&  defaultValue() const override;
        virtual const std::string&  minValue() const override;
        virtual const std::string&  maxValue() const override;

        void                        setProperties( Mlt::Properties* properties );

    private:
        std::string             m_identifier;
        std::string             m_name;
        std::string             m_type;
        std::string             m_description;
        std::string             m_defaultValue;
        std::string             m_minValue;
        std::string             m_maxValue;
    };
}
}

#endif // MLTPARAMETERINFO_H
