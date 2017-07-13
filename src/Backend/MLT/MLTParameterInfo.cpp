/*****************************************************************************
 * MLTParameterInfo.cpp:  Parameter information
 *****************************************************************************
 * Copyright (C) 2008-2017 VideoLAN
 *
 * Authors: Yikei Lu <luyikei.qmltu@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

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
