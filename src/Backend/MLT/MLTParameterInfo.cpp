#include "MLTParameterInfo.h"

#include <mlt++/MltProperties.h>

using namespace Backend::MLT;

inline std::string makeString( char* str )
{
    if ( str == nullptr )
        return std::string( "" );
    return std::string( str );
}

const std::string&
MLTParameterInfo::identifier() const
{
    return m_identifier;
}

const std::string&
MLTParameterInfo::name() const
{
    return m_name;
}

const std::string&
MLTParameterInfo::type() const
{
    return m_type;
}

const std::string&
MLTParameterInfo::description() const
{
    return m_description;
}

const std::string&
MLTParameterInfo::defaultValue() const
{
    return m_defaultValue;
}

const std::string&
MLTParameterInfo::minValue() const
{
    return m_minValue;
}

const std::string&
MLTParameterInfo::maxValue() const
{
    return m_maxValue;
}

void
MLTParameterInfo::setProperties( Mlt::Properties* properties )
{
    if ( properties == nullptr )
        return;
    m_identifier    = makeString( properties->get( "identifier" ) );
    m_name          = makeString( properties->get( "title" ) );
    m_type          = makeString( properties->get( "type" ) );
    m_description   = makeString( properties->get( "description" ) );
    m_defaultValue  = makeString( properties->get( "default" ) );
    m_minValue      = makeString( properties->get( "minimum" ) );
    m_maxValue      = makeString( properties->get( "maximum" ) );
}
