/*****************************************************************************
 * MLTFIlter.cpp:  Wrapper of Mlt::Filter
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

#include "MLTFilter.h"
#include <mlt++/MltFilter.h>
#include <mlt++/MltProducer.h>

#include <cstring>
#include "Backend/IBackend.h"
#include "MLTProfile.h"
#include "MLTInput.h"

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

MLTFilterInfo::~MLTFilterInfo()
{
    for ( IParameterInfo* info : m_paramInfos )
        delete info;
    m_paramInfos.clear();
}

const std::string&
MLTFilterInfo::identifier() const
{
    return m_identifier;
}

const std::string&
MLTFilterInfo::name() const
{
    return m_name;
}

const std::string&
MLTFilterInfo::description() const
{
    return m_description;
}

const std::string&
MLTFilterInfo::author() const
{
    return m_author;
}

const std::vector<Backend::IParameterInfo*>&
MLTFilterInfo::paramInfos() const
{
    return m_paramInfos;
}

void
MLTFilterInfo::setProperties( Mlt::Properties* properties )
{
    if ( properties == nullptr )
        return;

    m_identifier    = makeString( properties->get( "identifier" ) );
    m_name          = makeString( properties->get( "title" ) );
    m_description   = makeString( properties->get( "description" ) );
    m_author        = makeString( properties->get( "creator" ) );

    Mlt::Properties params( (mlt_properties)properties->get_data( "parameters" ) );

    for ( int i = 0; i < params.count(); ++i )
    {
        int s;
        Mlt::Properties param( (mlt_properties)params.get_data( i, s ) );
        MLTParameterInfo* info = new MLTParameterInfo;
        info->setProperties( &param );
        m_paramInfos.push_back( info );
    }
}

MLTFilter::MLTFilter( Backend::IProfile& profile, const char* id )
    : m_connectedProducer( nullptr )
{
    MLTProfile& mltProfile = static_cast<MLTProfile&>( profile );
    m_filter = new Mlt::Filter( *mltProfile.m_profile, id );
    m_service = m_filter;
    if ( isValid() == false )
        throw InvalidServiceException();
}

MLTFilter::MLTFilter( const char *id )
    : MLTFilter( Backend::instance()->profile(), id )
{

}

MLTFilter::MLTFilter( Mlt::Filter* filter, Mlt::Producer* connectedProducer )
{
    m_filter = filter;
    m_connectedProducer = connectedProducer;
    m_service = filter;
}

MLTFilter::~MLTFilter()
{
    delete m_filter;
}

std::string
MLTFilter::identifier() const
{
    return m_filter->get( "mlt_service" );
}

bool
MLTFilter::connect( Backend::IInput& input, int index )
{
    MLTInput* mltInput = dynamic_cast<MLTInput*>( &input );

    if ( mltInput == nullptr )
        return true;

    m_connectedProducer = mltInput->m_producer;

    return m_filter->connect( *mltInput->m_producer, index );
}

void
MLTFilter::setBoundaries( int64_t begin, int64_t end )
{
    m_filter->set_in_and_out( (int)begin, (int)end );
}

int64_t
MLTFilter::begin() const
{
    return m_filter->get_in();
}

int64_t
MLTFilter::end() const
{
    return m_filter->get_out();
}

int64_t
MLTFilter::length() const
{
    auto length = m_filter->get_length();

    if ( length == 0 && m_connectedProducer != nullptr )
        length = m_connectedProducer->get_playtime();

    return length ? length : MLTInput::Unlimited;
}

std::shared_ptr<Backend::IInput>
MLTFilter::input() const
{
    return std::make_shared<MLTInput>( m_connectedProducer );
}

const Backend::IFilterInfo&
MLTFilter::filterInfo() const
{
    return *Backend::instance()->filterInfo( identifier() );
}
