/*****************************************************************************
 * VLCInstance.cpp: Binding for libvlc instances
 *****************************************************************************
 * Copyright (C) 2008-2010 VideoLAN
 *
 * Authors: Hugo Beauzee-Luyssen <hugo@vlmc.org>
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
#include "VlmcDebug.h"

using namespace LibVLCpp;

Instance::Instance( QObject* parent /*= NULL*/ ) : QObject( parent )
{
    char const *argv[] =
    {
        "", //Keep this array entry empty. It will be replaced later if required.
//        "--ffmpeg-debug", "3",
        "--no-skip-frames",
        "--text-renderer", "dummy",
        "--vout", "dummy",
        "--no-video-title-show",            // Don't display the filename
        "--no-stats",                       // Don't display stats
        "--no-sub-autodetect-file",         // Don't detect subtitles
        //"--no-audio",
        //"--plugin-path", VLC_TREE "/modules",
        "--no-disable-screensaver", //No need to disable the screensaver, and save a thread.
//        "--no-overlay",
    };
    int argc = sizeof( argv ) / sizeof( *argv );

    int     debugLevel = VLMC_GET_INT( "private/LogLevel" );
    if ( debugLevel == VlmcDebug::Debug )
        argv[0] = "-vv";
    else if ( debugLevel == VlmcDebug::Verbose )
        argv[0] = "-v";

    m_internalPtr = libvlc_new( argc, argv );
    Q_ASSERT_X( m_internalPtr != NULL, "LibVLCpp::Instance::Instance()",
                "Can't launch VLMC without a valid LibVLC instance. Please check your VLC installation" );
}

Instance::~Instance()
{
    libvlc_release( m_internalPtr );
}
