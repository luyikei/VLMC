/*****************************************************************************
 * MLTTractor.cpp:  Wrapper of Mlt::Tractor
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

#include "MLTTractor.h"

#include <mlt++/MltTractor.h>
#include "MLTProfile.h"
#include "MLTBackend.h"
#include "MLTTransition.h"
#include "MLTFilter.h"
#include "Tools/VlmcDebug.h"

#include <cassert>

using namespace Backend::MLT;

MLTTractor::MLTTractor()
    : MLTTractor( Backend::instance()->profile() )
{
}

MLTTractor::MLTTractor( Backend::IProfile& profile )
    : MLTInput()
{
    MLTProfile& mltProfile = static_cast<MLTProfile&>( profile );
    m_tractor  = new Mlt::Tractor( *mltProfile.m_profile );
    m_producer = m_tractor;
    m_service  = m_tractor;
}

MLTTractor::~MLTTractor()
{
    m_producer = nullptr;
    delete m_tractor;
}

void
MLTTractor::refresh()
{
    m_tractor->refresh();
}

bool
MLTTractor::setTrack( Backend::IInput& input, int index )
{
    MLTInput* mltInput = dynamic_cast<MLTInput*>( &input );
    assert( mltInput );
    return m_tractor->set_track( *mltInput->m_producer, index );
}

bool
MLTTractor::insertTrack( Backend::IInput& input, int index )
{
    MLTInput* mltInput = dynamic_cast<MLTInput*>( &input );
    assert( mltInput );
    return m_tractor->insert_track( *mltInput->m_producer, index );
}

bool
MLTTractor::removeTrack( int index )
{
    return m_tractor->remove_track( index );
}

Backend::IInput*
MLTTractor::track( int index ) const
{
    return new MLTInput( m_tractor->track( index ), nullptr );
}

int
MLTTractor::count() const
{
    return m_tractor->count();
}

void
MLTTractor::addTransition( Backend::ITransition& transition, int aTrack, int bTrack )
{
    MLTTransition* mltTransition = dynamic_cast<MLTTransition*>( &transition );
    m_tractor->plant_transition( mltTransition->m_transition, aTrack, bTrack );
}

void
MLTTractor::addFilter( Backend::IFilter& filter, int track )
{
    MLTFilter* mltFilter = dynamic_cast<MLTFilter*>( &filter );
    m_tractor->plant_filter( mltFilter->m_filter, track );
}

bool
MLTTractor::connect( Backend::IInput& input )
{
    MLTInput* mltInput = dynamic_cast<MLTInput*>( &input );
    assert( mltInput );
    return m_tractor->connect( *mltInput->m_producer );
}
