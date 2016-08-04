/*****************************************************************************
 * MLTProfile.cpp:  Wrapper of Mlt::Profile
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

#include "MLTProfile.h"

#include <mlt++/MltProfile.h>

using namespace Backend::MLT;

MLTProfile::MLTProfile()
    : m_profile( new Mlt::Profile )
{

}

MLTProfile::MLTProfile( Mlt::Profile* profile )
    : m_profile( profile )
{

}

MLTProfile::~MLTProfile()
{
    delete m_profile;
}

int
MLTProfile::frameRateNum() const
{
    return m_profile->frame_rate_num();
}

int
MLTProfile::frameRateDen() const
{
    return m_profile->frame_rate_den();
}

double
MLTProfile::fps() const
{
    return m_profile->fps();
}

int
MLTProfile::width() const
{
    return m_profile->width();
}

int
MLTProfile::height() const
{
    return m_profile->height();
}

int
MLTProfile::aspectRatioNum() const
{
    return m_profile->display_aspect_num();
}

int
MLTProfile::aspectRatioDen() const
{
    return m_profile->display_aspect_den();
}

double
MLTProfile::aspectRatio() const
{
    return m_profile->dar();
}

void
MLTProfile::setWidth( int width )
{
    m_profile->set_width( width );;
}

void
MLTProfile::setHeight( int height )
{
    m_profile->set_height( height );
}

void
MLTProfile::setAspectRatio( int numerator, int denominator )
{
    m_profile->set_display_aspect( numerator, denominator );
}

void
MLTProfile::setFrameRate( int numerator, int denominator )
{
    m_profile->set_frame_rate( numerator, denominator );
}
