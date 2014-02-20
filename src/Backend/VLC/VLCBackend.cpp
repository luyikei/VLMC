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
#include "SettingsManager.h"
#include "VlmcDebug.h"
#include "VlmcLogger.h"

using namespace Backend;
using namespace Backend::VLC;

IBackend *Backend::getBackend()
{
    return VLCBackend::getInstance();
}

VLCBackend::VLCBackend()
{
    QVector<const char*> argv;
    argv << "--no-skip-frames"
        // << "--ffmpeg-debug", "3",
        << "--text-renderer" << "dummy"
        << "--no-sub-autodetect-file"           // Don't detect subtitles
        // << "--no-audio"
        // << "--no-overlay",
        << "--no-disable-screensaver";             //No need to disable the screensaver, and save a thread.

    int     debugLevel = VLMC_GET_INT( "private/VlcLogLevel" );
    if ( debugLevel == VlmcLogger::Debug )
        argv << "-vv";
    else if ( debugLevel == VlmcLogger::Verbose )
        argv << "-v";
    m_vlcInstance = new LibVLCpp::Instance( argv.count(), &argv.front() );
    m_vlcInstance->setLogHook( this, &logHook );
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

LibVLCpp::Instance*
VLCBackend::vlcInstance()
{
    return m_vlcInstance;
}

void
VLCBackend::logHook(void *data, int level, const libvlc_log_t *ctx, const char *fmt, va_list args)
{
    Q_UNUSED( data )
    Q_UNUSED( ctx )

    char* msg;
    if (vasprintf( &msg, fmt, args ) < 0 )
        return;
    if ( level <= LIBVLC_NOTICE )
        vlmcDebug() << "[VLC]" << msg;
    else if ( level == LIBVLC_WARNING )
        vlmcWarning() << "[VLC]" << msg;
    else if ( level == LIBVLC_ERROR )
        vlmcCritical() << "[VLC]" << msg;
    else
    {
        vlmcCritical() << "Unexpected logging level for VLC log" << level;
        vlmcCritical() << "[VLC]" << msg;
    }
    free( msg );
}
