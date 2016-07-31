/*****************************************************************************
 * MLTBackend.cpp: Backend implementation consisting of
 *                 Mlt::Factory, Mlt::Profile and Mlt::Repository
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

#include "MLTBackend.h"

#include <mlt++/MltFactory.h>
#include <mlt++/MltProperties.h>
#include <mlt++/MltRepository.h>
#include <mlt++/MltService.h>

#include <mlt/framework/mlt_log.h>

#include "MLTFilter.h"

#include <mutex>
#include <sstream>

#include "Tools/VlmcDebug.h"
#include "vlmc.h"

using namespace Backend;
using namespace Backend::MLT;


static Backend::IBackend::LogHandler    staticLogHandler;
static std::mutex                       logMutex;

IBackend*
Backend::instance()
{
    return MLTBackend::instance();
}

MLTBackend::MLTBackend()
{
    m_mltRepo = Mlt::Factory::init();
    m_profile.setFrameRate( 2997, 100 );

    for ( int i = 0; i < m_mltRepo->filters()->count(); ++i )
    {
        auto pro = std::unique_ptr<Mlt::Properties>( m_mltRepo->metadata( filter_type, m_mltRepo->filters()->get_name( i ) ) );
        auto filterInfo = new MLTFilterInfo;
        filterInfo->setProperties( pro.get() );
        if ( filterInfo->identifier().empty() == true )
        {
            delete filterInfo;
            continue;
        }
        m_availableFilters[ filterInfo->identifier() ] = filterInfo;
    }
}

MLTBackend::~MLTBackend()
{
    Mlt::Factory::close();

    for ( auto info : m_availableFilters )
        delete info.second;
}

IProfile&
MLTBackend::profile()
{
    return m_profile;
}

const std::map<std::string, IFilterInfo*>&
MLTBackend::availableFilters() const
{
    return m_availableFilters;
}

IFilterInfo*
MLTBackend::filterInfo( const std::string& id ) const
{
    auto it = m_availableFilters.find( id );
    if ( it != m_availableFilters.end() )
        return (*it).second;
    return nullptr;
}

void
MLTBackend::setLogHandler( IBackend::LogHandler logHandler )
{
    staticLogHandler = logHandler;
    mlt_log_set_callback( []( void* ptr, int level, const char* format, va_list vl )
    {
        std::string msg_str;

        Mlt::Service service( ( mlt_service )ptr );

        if ( service.is_valid() )
        {
            const char *mlt_type = service.get( "mlt_type" );
            const char *mlt_service = service.get( "mlt_service" );
            std::ostringstream os;

            if ( mlt_service && mlt_type )
                os << "[" << mlt_type << " " << mlt_service << "] ";
            else if ( mlt_type )
            {
                os << "[" << mlt_type << " " << ptr << "] ";
            }

            msg_str = os.str();
        }

        char* buffer = nullptr;

        auto lvl = IBackend::None;
        if ( MLT_LOG_INFO <= level )
            lvl = IBackend::Debug;
        else if ( MLT_LOG_WARNING <= level )
            lvl = IBackend::Warning;
        else
            lvl = IBackend::Error;

        std::lock_guard<std::mutex> lock( logMutex );

        if ( vasprintf( &buffer, format, vl ) < 0 )
            return;

        if ( buffer )
            msg_str += buffer;

        if ( !msg_str.empty() )
            staticLogHandler( lvl, msg_str.c_str() );

        free( buffer );
    } );
}
