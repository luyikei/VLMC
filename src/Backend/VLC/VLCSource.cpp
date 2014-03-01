/*****************************************************************************
 * VLCSource.cpp: Implementation of ISource based on a libvlc_media_t
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

#include "EventWaiter.h"
#include "VLCBackend.h"
#include "VLCSource.h"
#include "VLCVmemRenderer.h"
#include "Tools/VlmcDebug.h"

using namespace Backend;
using namespace Backend::VLC;

VLCSource::VLCSource( VLCBackend* backend, const QString& path )
    : m_backend( backend )
    , m_width( 0 )
    , m_height( 0 )
    , m_fps( .0f )
    , m_nbVideoTracks( 0 )
    , m_nbAudioTracks( 0 )
    , m_length( 0 )
    , m_snapshot( NULL )
    , m_isParsed( false )
    , m_nbFrames( 0 )
{
    char buffer[256];
    sprintf( buffer, "file://%s", qPrintable( path ) );
    m_media = new LibVLCpp::Media( backend->vlcInstance(), buffer );
}

VLCSource::~VLCSource()
{
    delete m_media;
}

LibVLCpp::Media*
VLCSource::media()
{
    return m_media;
}

ISourceRenderer*
VLCSource::createRenderer( ISourceRendererEventCb *callback )
{
    return new VLCSourceRenderer( m_backend, this, callback );
}

bool
VLCSource::preparse()
{
    // This assume we won't try to parse the same media twice ast the same time
    m_isParsed = true;

    VmemRenderer*           renderer = new VmemRenderer( m_backend, this, NULL );
    LibVLCpp::MediaPlayer*  mediaPlayer = renderer->mediaPlayer();
    {
        EventWaiter ew( mediaPlayer, true );
        ew.add( libvlc_MediaPlayerVout );
        renderer->start();
        if ( ew.wait( 3000 ) != EventWaiter::Success )
        {
            delete renderer;
            return false;
        }
    }
    m_media->parse();
    m_nbVideoTracks = mediaPlayer->getNbVideoTrack();
    m_nbAudioTracks = mediaPlayer->getNbAudioTrack();
    //FIXME: handle images with something like m_length = 10000;
    m_length = mediaPlayer->getLength();
    if ( hasVideo() == true )
    {
        mediaPlayer->getSize( &m_width, &m_height );
        m_fps = mediaPlayer->getFps();
        if ( m_fps < 0.1f )
        {
            vlmcWarning() << "Invalid FPS for source" << m_media->mrl();
            delete renderer;
            return false;
        }
        m_nbFrames = (int64_t)( (float)( m_length / 1000 ) * m_fps );
        return computeSnapshot( renderer );
    }
    delete renderer;
    return true;
}

bool
VLCSource::isParsed() const
{
    return m_isParsed;
}

static bool
checkTimeChanged( const libvlc_event_t* event )
{
    Q_ASSERT( event->type == libvlc_MediaPlayerPositionChanged );
    return ( event->u.media_player_position_changed.new_position > 0.2f );
}

bool
VLCSource::computeSnapshot( VmemRenderer* renderer )
{
    Q_ASSERT( m_snapshot == NULL );
    LibVLCpp::MediaPlayer*  mediaPlayer = renderer->mediaPlayer();
    {
        EventWaiter ew( mediaPlayer, false );
        ew.add( libvlc_MediaPlayerPositionChanged );
        ew.setValidationCallback( &checkTimeChanged );
        renderer->setTime( m_length / 3 );
        if ( ew.wait( 3000 ) != EventWaiter::Success )
        {
            delete renderer;
            return false;
        }
    }
    m_snapshot = renderer->waitSnapshot();
    delete renderer;
    return m_snapshot != NULL;
}

unsigned int
VLCSource::width() const
{
    return m_width;
}

unsigned int
VLCSource::height() const
{
    return m_height;
}

int64_t
VLCSource::length() const
{
    return m_length;
}

float
VLCSource::fps() const
{
    return m_fps;
}

bool
VLCSource::hasVideo() const
{
    return m_nbVideoTracks > 0;
}

unsigned int
VLCSource::nbVideoTracks() const
{
    return m_nbVideoTracks;
}

bool VLCSource::hasAudio() const
{
    return m_nbAudioTracks > 0;
}

unsigned int
VLCSource::nbAudioTracks() const
{
    return m_nbAudioTracks;
}

const uint8_t*
VLCSource::snapshot() const
{
    if ( hasVideo() == false || m_snapshot == NULL )
        return NULL;
    return m_snapshot->bits();
}

int64_t
VLCSource::nbFrames() const
{
    return m_nbFrames;
}

