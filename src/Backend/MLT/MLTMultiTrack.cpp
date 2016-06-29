/*****************************************************************************
 * MLTMultiTrack.cpp:  Wrapper of Mlt::Tractor
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

#include "MLTMultiTrack.h"

#include <mlt++/MltTractor.h>
#include "MLTProfile.h"
#include "MLTBackend.h"
#include "MLTTransition.h"
#include "MLTFilter.h"
#include "Tools/VlmcDebug.h"

#include <cassert>

using namespace Backend::MLT;

MLTMultiTrack::MLTMultiTrack()
    : MLTMultiTrack( Backend::instance()->profile() )
{
}

MLTMultiTrack::MLTMultiTrack( Backend::IProfile& profile )
    : MLTInput()
{
    MLTProfile& mltProfile = static_cast<MLTProfile&>( profile );
    m_tractor  = new Mlt::Tractor( *mltProfile.m_profile );
}

MLTMultiTrack::~MLTMultiTrack()
{
    delete m_tractor;
}

Mlt::Tractor*
MLTMultiTrack::tractor()
{
    return m_tractor;
}

Mlt::Tractor*
MLTMultiTrack::tractor() const
{
    return m_tractor;
}

Mlt::Producer*
MLTMultiTrack::producer()
{
    return tractor();
}

Mlt::Producer*
MLTMultiTrack::producer() const
{
    return tractor();
}

void
MLTMultiTrack::refresh()
{
    tractor()->refresh();
}

bool
MLTMultiTrack::setTrack( Backend::IInput& input, int index )
{
    MLTInput* mltInput = dynamic_cast<MLTInput*>( &input );
    assert( mltInput );
    return tractor()->set_track( *mltInput->producer(), index );
}

bool
MLTMultiTrack::insertTrack( Backend::IInput& input, int index )
{
    MLTInput* mltInput = dynamic_cast<MLTInput*>( &input );
    assert( mltInput );
    return tractor()->insert_track( *mltInput->producer(), index );
}

bool
MLTMultiTrack::removeTrack( int index )
{
    return tractor()->remove_track( index );
}

Backend::IInput*
MLTMultiTrack::track( int index ) const
{
    return new MLTInput( tractor()->track( index ), nullptr );
}

int
MLTMultiTrack::count() const
{
    return tractor()->count();
}

void
MLTMultiTrack::addTransition( Backend::ITransition& transition, int aTrack, int bTrack )
{
    MLTTransition* mltTransition = dynamic_cast<MLTTransition*>( &transition );
    tractor()->plant_transition( mltTransition->transition(), aTrack, bTrack );
}

void
MLTMultiTrack::addFilter( Backend::IFilter& filter, int track )
{
    MLTFilter* mltFilter = dynamic_cast<MLTFilter*>( &filter );
    tractor()->plant_filter( mltFilter->filter(), track );
}

bool
MLTMultiTrack::connect( Backend::IInput& input )
{
    MLTInput* mltInput = dynamic_cast<MLTInput*>( &input );
    assert( mltInput );
    return tractor()->connect( *mltInput->producer() );
}
