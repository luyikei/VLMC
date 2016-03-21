/*****************************************************************************
 * VLCBackend.cpp: Provides VLC functionnalities through IBackend interface
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

#include <QVector>

#include "VLCBackend.h"
#include "VLCSource.h"
#include "VLCMemorySource.h"
#include "Tools/VlmcDebug.h"
#include "Tools/VlmcLogger.h"

using namespace Backend;
using namespace Backend::VLC;

IBackend *Backend::getBackend()
{
    return VLCBackend::getInstance();
}

VLCBackend::VLCBackend()
{
    const char* argv[] =
    {
        "--no-skip-frames",
        // "--ffmpeg-debug", "3",
        "--text-renderer", "dummy",
        "--no-sub-autodetect-file",     // Don't detect subtitles
        // "--no-audio"
        // "--no-overlay",
        "--no-disable-screensaver",     //No need to disable the screensaver, and save a thread.
    };

    m_vlcInstance = ::VLC::Instance( sizeof( argv ) / sizeof( argv[0] ), argv );
}

ISource*
VLCBackend::createSource(const char *path)
{
    return new VLCSource( this, path );
}

IMemorySource*
VLCBackend::createMemorySource()
{
    return new VLCMemorySource( this );
}

void
VLCBackend::setLogHandler( IBackend::LogHandler logHandler )
{
    m_vlcInstance.logUnset();
    if ( (bool)logHandler == true )
    {
        m_vlcInstance.logSet( [logHandler]( int level, const void*, const std::string& msg ) {
            auto lvl = IBackend::None;
            if ( level == LIBVLC_NOTICE || level == LIBVLC_DEBUG )
                lvl = IBackend::Debug;
            else if ( level <= LIBVLC_WARNING )
                lvl = IBackend::Warning;
            else
                lvl = IBackend::Error;
            logHandler( lvl, msg.c_str() );
        });
    }
}

::VLC::Instance&
VLCBackend::vlcInstance()
{
    return m_vlcInstance;
}
