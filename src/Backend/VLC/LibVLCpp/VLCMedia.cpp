/*****************************************************************************
 * VLCMedia.cpp: Binding for libvlc_media
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

#include "VLCMedia.h"
#include "VLCInstance.h"

//Allow PRId64 to be defined:
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <cstdio>

using namespace LibVLCpp;

Media::Media(LibVLCpp::Instance* instance, const char *filename )
    : m_tracks( NULL )
    , m_nbTracks( 0 )
    , m_mrl( NULL )

{
    m_internalPtr = libvlc_media_new_location( *instance, filename);
}

Media::~Media()
{
    if ( m_tracks != NULL )
        libvlc_media_tracks_release( m_tracks, m_nbTracks );
    libvlc_media_release( m_internalPtr );
    free( m_mrl );
}

void
Media::addOption( const char* opt )
{
    libvlc_media_add_option_flag( m_internalPtr, opt, libvlc_media_option_trusted );
}

void
Media::setVideoLockCallback( void* callback )
{
    char    param[64];
    sprintf( param, ":sout-smem-video-prerender-callback=%"PRId64, (intptr_t)callback );
    addOption(param);
}

void
Media::setVideoUnlockCallback( void* callback )
{
    char    param[64];
    sprintf( param, ":sout-smem-video-postrender-callback=%"PRId64, (intptr_t)callback );
    addOption( param );
}

void
Media::setAudioLockCallback( void* callback )
{
    char    param[64];
    sprintf( param, ":sout-smem-audio-prerender-callback=%"PRId64, (intptr_t)callback );
    addOption(param);
}

void
Media::setAudioUnlockCallback( void* callback )
{
    char    param[64];
    sprintf( param, ":sout-smem-audio-postrender-callback=%"PRId64, (intptr_t)callback );
    addOption( param );
}

void
Media::setVideoDataCtx( void* dataCtx )
{
    char    param[64];

    sprintf( param, ":sout-smem-video-data=%"PRId64, (intptr_t)dataCtx );
    addOption( param );
}

void
Media::setAudioDataCtx( void* dataCtx )
{
    char    param[64];

    sprintf( param, ":sout-smem-audio-data=%"PRId64, (intptr_t)dataCtx );
    addOption( param );
}

const char*
Media::mrl()
{
    if ( m_mrl == NULL )
        m_mrl = libvlc_media_get_mrl( m_internalPtr );
    return m_mrl;
}

void
Media::parse()
{
    libvlc_media_parse( *this );
}

void
Media::fetchTrackInfo()
{
    m_nbTracks = libvlc_media_tracks_get( *this, &m_tracks );
}

unsigned int
Media::videoCodec() const
{
    for ( int i = 0; i < m_nbTracks; ++i )
        if ( m_tracks[i]->i_type == libvlc_track_video )
            return m_tracks[i]->i_codec;
    return 0;
}

unsigned int
Media::audioCodec() const
{
    for ( int i = 0; i < m_nbTracks; ++i )
        if ( m_tracks[i]->i_type == libvlc_track_video )
            return m_tracks[i]->i_codec;
    return 0;
}
