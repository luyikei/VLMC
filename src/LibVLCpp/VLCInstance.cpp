/*****************************************************************************
 * VLCInstance.cpp: Binding for libvlc instances
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

#include "VLCInstance.h"
#include "vlc/vlc.h"

#include "SettingsManager.h"
#include "VlmcLogger.h"

#include <QVector>

using namespace LibVLCpp;

Instance::Instance( QObject* parent /*= NULL*/ ) : QObject( parent )
{
    QVector<const char*> argv;
    argv << "--no-skip-frames"
        // << "--ffmpeg-debug", "3",
        << "--text-renderer" << "dummy"
        << "--no-sub-autodetect-file"           // Don't detect subtitles
        // << "--no-audio"
        // << "--no-overlay",
        << "--no-disable-screensaver";             //No need to disable the screensaver, and save a thread.

    int     debugLevel = VLMC_GET_INT( "private/LogLevel" );
    if ( debugLevel == VlmcLogger::Debug )
        argv << "-vv";
    else if ( debugLevel == VlmcLogger::Verbose )
        argv << "-v";

    m_internalPtr = libvlc_new( argv.count(), &argv.front() );
    Q_ASSERT_X( m_internalPtr != NULL, "LibVLCpp::Instance::Instance()",
                "Can't launch VLMC without a valid LibVLC instance. Please check your VLC installation" );
}

Instance::~Instance()
{
    libvlc_release( m_internalPtr );
}
