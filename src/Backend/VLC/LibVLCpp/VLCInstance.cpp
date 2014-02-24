/*****************************************************************************
 * VLCInstance.cpp: Binding for libvlc instances
 *****************************************************************************
 * Copyright (C) 2008-2014 VideoLAN
 *
 * Authors: Hugo Beauzée-Luyssen <hugo@beauzee.fr>
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

#include "VLCInstance.h"
#include "vlc/vlc.h"

#include <cstdio>
#include <cassert>

using namespace LibVLCpp;

Instance::Instance(int argc, const char **argv)
{
    m_internalPtr = libvlc_new( argc, argv );
    assert( m_internalPtr != NULL );
}

Instance::~Instance()
{
    libvlc_release( m_internalPtr );
}

void
Instance::setLogHook( void* data, libvlc_log_cb hook )
{
    libvlc_log_set( m_internalPtr, hook, data );

}

