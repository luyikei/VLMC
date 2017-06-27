/*****************************************************************************
 * MLTService.cpp:  Wrapper of Mlt::Service
 *****************************************************************************
 * Copyright (C) 2008-2016 VideoLAN
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

#include "MLTService.h"
#include <mlt++/MltService.h>
#include <mlt++/MltFilter.h>

#include "MLTFilter.h"
#include "MLTProfile.h"
#include "MLTParameterInfo.h"
#include <cassert>

using namespace Backend::MLT;

inline std::string makeString( char* str )
{
    if ( str == nullptr )
        return std::string( "" );
    return std::string( str );
}

MLTServiceInfo::~MLTServiceInfo()
{
    for ( IParameterInfo* info : m_paramInfos )
        delete info;
    m_paramInfos.clear();
}

const std::string&
MLTServiceInfo::identifier() const
{
    return m_identifier;
}

const std::string&
MLTServiceInfo::name() const
{
    return m_name;
}

const std::string&
MLTServiceInfo::description() const
{
    return m_description;
}

const std::string&
MLTServiceInfo::author() const
{
    return m_author;
}

const std::vector<Backend::IParameterInfo*>&
MLTServiceInfo::paramInfos() const
{
    return m_paramInfos;
}

void
MLTServiceInfo::setProperties( Mlt::Properties* properties )
{
    if ( properties == nullptr )
        return;

    m_identifier    = makeString( properties->get( "identifier" ) );
    m_name          = makeString( properties->get( "title" ) );
    m_description   = makeString( properties->get( "description" ) );
    m_author        = makeString( properties->get( "creator" ) );

    Mlt::Properties params( properties->get_data( "parameters" ) );

    for ( int i = 0; i < params.count(); ++i )
    {
        int s;
        Mlt::Properties param( params.get_data( i, s ) );
        MLTParameterInfo* info = new MLTParameterInfo;
        info->setProperties( &param );
        m_paramInfos.push_back( info );
    }
}

MLTService::~MLTService()
{

}

Mlt::Service*
MLTService::service()
{
    return m_service;
}

Mlt::Service*
MLTService::service() const
{
    return m_service;
}

std::string
MLTService::identifier() const
{
    return service()->get( "mlt_service" );
}

bool
MLTService::isValid() const
{
    return service()->is_valid();
}

Mlt::Properties*
MLTService::properties()
{
    return service();
}
