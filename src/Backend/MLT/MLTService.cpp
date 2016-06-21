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

#include "MLTService.h"
#include <mlt++/MltService.h>
#include <mlt++/MltFilter.h>

#include "MLTFilter.h"
#include "MLTProfile.h"
#include <cassert>

using namespace Backend::MLT;

MLTService::MLTService()
    : m_service( nullptr )
{

}

// Not intended to be created.
MLTService::MLTService( Mlt::Service* service )
    : m_service( service )
{

}

MLTService::~MLTService()
{

}

std::string
MLTService::identifier() const
{
    return m_service->get( "mlt_service" );
}

Backend::IService*
MLTService::consumer() const
{
    return new MLTService( m_service->consumer() );
}

Backend::IService*
MLTService::producer() const
{
    return new MLTService( m_service->producer() );
}

Backend::IProfile*
MLTService::profile() const
{
    return new MLTProfile( m_service->profile() );
}

bool
MLTService::attach( Backend::IFilter& filter )
{
    MLTFilter* mltFilter = dynamic_cast<MLTFilter*>( &filter );
    assert( mltFilter );
    return m_service->attach( *mltFilter->m_filter );
}

bool
MLTService::detach( Backend::IFilter& filter )
{
    MLTFilter* mltFilter = dynamic_cast<MLTFilter*>( &filter );
    assert( mltFilter );
    return m_service->detach( *mltFilter->m_filter );
}

bool
MLTService::detach( int index )
{
    auto filter = m_service->filter( index );
    auto ret = m_service->detach( *filter );
    delete filter;
    return ret;
}

int
MLTService::filterCount() const
{
    return m_service->filter_count();
}

bool
MLTService::moveFilter( int from, int to )
{
    return m_service->move_filter( from, to );
}

Backend::IFilter*
MLTService::filter( int index ) const
{
    return new MLTFilter( m_service->filter( index ) );
}

void
MLTService::setProfile( Backend::IProfile& profile )
{
    MLTProfile& mltProfile = static_cast<MLTProfile&>( profile );
    m_service->set_profile( *mltProfile.m_profile );
}

bool
MLTService::isValid() const
{
    return m_service->get_service() != nullptr;
}

Mlt::Properties*
MLTService::properties()
{
    return m_service;
}
