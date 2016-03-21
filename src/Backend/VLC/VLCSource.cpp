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
    m_media = ::VLC::Media( backend->vlcInstance(), path.toStdString(), ::VLC::Media::FromPath );
}

::VLC::Media&
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
    Q_ASSERT( m_nbAudioTracks == 0 );
    Q_ASSERT( m_nbVideoTracks == 0 );

    VmemRenderer renderer( m_backend, this, NULL );
    m_media.parse();
    m_length = m_media.duration();
    auto tracks = m_media.tracks();
    for ( const auto& t : tracks )
    {
        if ( t.type() == ::VLC::MediaTrack::Type::Video )
        {
            ++m_nbVideoTracks;
            //FIXME: This doesn't handle media with multiple video tracks
            //We assume the first track to be the most valuable one for now.
            if ( m_nbVideoTracks == 1 )
            {
                m_fps = (float)t.fpsNum() / (float)t.fpsDen();
                if ( m_fps < 0.1f )
                {
                    vlmcWarning() << "Invalid FPS for source" << m_media.mrl();
                    return false;
                }
                m_width = t.width();
                m_height = t.height();
                m_nbFrames = (int64_t)( (float)( m_length / 1000 ) * m_fps );
                computeSnapshot( renderer );
            }
        }
        else if ( t.type() == ::VLC::MediaTrack::Type::Audio )
            ++m_nbAudioTracks;
    }
    //FIXME: handle images with something like m_length = 10000;
    return true;
}

bool
VLCSource::isParsed() const
{
    return m_isParsed;
}

bool
VLCSource::computeSnapshot( VmemRenderer& renderer )
{
    Q_ASSERT( m_snapshot == NULL );
    {
        renderer.setTime( m_length / 3 );
        //FIXME: This is bad and you should feel bad.
        QThread::usleep( 500000 );
    }
    renderer.start();
    m_snapshot = renderer.waitSnapshot();
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

