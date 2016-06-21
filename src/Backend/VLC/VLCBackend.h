/*****************************************************************************
 * VLCBackend.h: Provides VLC functionnalities through IBackend interface
 *****************************************************************************
 * Copyright (C) 2008-2016 VideoLAN
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

#include "Tools/Singleton.hpp"
#include "vlcpp/vlc.hpp"

namespace Backend
{
namespace VLC
{
class VLCMemorySource;
class VLCSource;
class VLCBackend : public Singleton<VLCBackend>
{
    public:
        enum LogLevel
        {
            Debug,
            Warning,
            Error,
            None
        };
        using LogHandler = std::function<void( LogLevel logLevel, const char* msg )>;

        VLCBackend();
        virtual VLCSource*        createSource( const char* path );
        virtual VLCMemorySource*  createMemorySource();
        virtual void            setLogHandler( LogHandler logHandler );

        // Accessible from VLCBackend only:
        ::VLC::Instance&        vlcInstance();

    private:
        friend class Singleton<VLCBackend>;
        ::VLC::Instance     m_vlcInstance;
};

} //VLC

VLC::VLCBackend* getVLCBackend();


} //Backend

#endif // VLCBACKEND_H
