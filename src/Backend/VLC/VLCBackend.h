/*****************************************************************************
 * VLCBackend.h: Provides VLC functionnalities through IBackend interface
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

#ifndef VLCBACKEND_H
#define VLCBACKEND_H

#include <cstdarg>

#include "Backend/IBackend.h"
#include "Tools/Singleton.hpp"
#include "libvlcpp/vlcpp/vlc.hpp"

namespace Backend
{
namespace VLC
{

class VLCBackend : public IBackend, public Singleton<VLCBackend>
{
    public:
        VLCBackend();
        virtual ISource*        createSource( const char* path );
        virtual IMemorySource*  createMemorySource();
        virtual void            setLogHandler( LogHandler logHandler );

        // Accessible from VLCBackend only:
        ::VLC::Instance&        vlcInstance();

    private:
        friend class Singleton<VLCBackend>;
        ::VLC::Instance     m_vlcInstance;
};

} //VLC

IBackend* getBackend();


} //Backend

#endif // VLCBACKEND_H
