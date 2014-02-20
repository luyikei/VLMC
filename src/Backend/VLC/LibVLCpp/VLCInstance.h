/*****************************************************************************
 * VLCInstance.h: Binding for libvlc instance
 *****************************************************************************
 * Copyright (C) 2008-2010 VideoLAN
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

#ifndef VLCINSTANCE_H
#define VLCINSTANCE_H

#include "VLCpp.hpp"
#include "Singleton.hpp"

struct libvlc_instance_t;

namespace LibVLCpp
{
    /**
     *  \warning    This class should be released after every other LibVLCpp classes.
     */
    class   Instance : public Internal< libvlc_instance_t >
    {
    public:
        Instance( int argc, const char** argv );
        ~Instance();
        void setLogHook( void *data, libvlc_log_cb hook );
    };
}

#endif // VLCINSTANCE_H
