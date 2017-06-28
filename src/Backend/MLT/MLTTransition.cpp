/*****************************************************************************
 * MLTTransition.cpp:  Wrapper of Mlt::Transition
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

#include "MLTTransition.h"

#include <mlt++/MltTransition.h>
#include "MLTProfile.h"
#include "MLTBackend.h"

using namespace Backend::MLT;

MLTTransition::MLTTransition( Backend::IProfile& profile, const char *id )
{
    MLTProfile& mltProfile = static_cast<MLTProfile&>( profile );
    m_transition = new Mlt::Transition( *mltProfile.m_profile, id );
    if ( isValid() == false )
        throw InvalidServiceException();
}

MLTTransition::MLTTransition( const char* id )
    : MLTTransition( Backend::instance()->profile(), id )
{

}

MLTTransition::~MLTTransition()
{
    delete m_transition;
}

Mlt::Transition*
MLTTransition::transition()
{
    return m_transition;
}

Mlt::Transition*
MLTTransition::transition() const
{
    return m_transition;
}

Mlt::Service*
MLTTransition::service()
{
    return transition();
}

Mlt::Service*
MLTTransition::service() const
{
    return transition();
}

void
MLTTransition::setBoundaries( int64_t begin, int64_t end )
{
    transition()->set_in_and_out( (int)begin, (int)end );
}

int64_t
MLTTransition::begin() const
{
    return transition()->get_in();
}

int64_t
MLTTransition::end() const
{
    return transition()->get_out();
}

int64_t
MLTTransition::length() const
{
    return transition()->get_length();
}
