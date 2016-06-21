/*****************************************************************************
 * MLTTrack.cpp:  Wrapper of Mlt::Track
 *****************************************************************************
 * Copyright (C) 2008-2016 VideoLAN
 *
 * Authors: Yikei Lu <luyikei.qmltu@gmail.com>
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

#include "MLTTrack.h"
#include "MLTProfile.h"
#include "MLTBackend.h"

#include <mlt++/MltPlaylist.h>

#include <cassert>
#include <cstring>

using namespace Backend::MLT;

enum HideType
{
    None,
    Video,
    Audio,
    VideoAndAudio
};

MLTTrack::MLTTrack()
    : MLTTrack( Backend::instance()->profile() )
{
}

MLTTrack::MLTTrack( IProfile &profile )
    : MLTProducer()
{
    MLTProfile& mltProfile = static_cast<MLTProfile&>( profile );
    m_playlist = new Mlt::Playlist( *mltProfile.m_profile );
    m_producer = m_playlist;
    m_service = m_playlist;
}

MLTTrack::~MLTTrack()
{
    m_producer = nullptr;
    delete m_playlist;
}

bool
MLTTrack::insertAt( Backend::IProducer& producer, int64_t startFrame )
{
    auto mltProducer = dynamic_cast<MLTProducer*>( &producer );
    assert( mltProducer );
    return m_playlist->insert_at( (int)startFrame, mltProducer->m_producer, 1 );
}

bool
MLTTrack::append( Backend::IProducer& producer )
{
    auto mltProducer = dynamic_cast<MLTProducer*>( &producer );
    assert( mltProducer );
    return m_playlist->append( *mltProducer->m_producer );
}

bool
MLTTrack::remove( int index )
{
    auto ret = m_playlist->remove( index );

    // Remove last blanks.
    for ( int i = m_playlist->count() - 1; i >= 0; --i )
    {
        if ( strcmp( m_playlist->clip_info( i )->resource, "blank" ) == 0 )
            m_playlist->remove( i );
        else
            break;
    }

    return ret;
}

bool
MLTTrack::move( int src, int dist )
{
    return m_playlist->move( src, dist );
}

Backend::IProducer*
MLTTrack::clip( int index ) const
{
    return new MLTProducer( m_playlist->get_clip( index ) );
}

Backend::IProducer*
MLTTrack::clipAt( int64_t position ) const
{
    return new MLTProducer( m_playlist->get_clip_at( (int)position ) );
}

bool
MLTTrack::resizeClip( int clip, int64_t begin, int64_t end )
{
    return m_playlist->resize_clip( clip, (int)begin, (int)end );
}

int
MLTTrack::clipIndexAt( int64_t position )
{
    return m_playlist->get_clip_index_at( (int)position );
}

int
MLTTrack::count() const
{
    return m_playlist->count();
}

void
MLTTrack::clear()
{
    m_playlist->clear();
}

void
MLTTrack::setAudioOutput( bool enabled )
{
    if ( enabled == true )
        if ( m_playlist->get_int( "hide" ) == HideType::VideoAndAudio )
            m_playlist->set( "hide", HideType::Video );
        else
            m_playlist->set( "hide", HideType::None );
    else
        if ( m_playlist->get_int( "hide" ) == HideType::Video )
            m_playlist->set( "hide", HideType::VideoAndAudio );
        else
            m_playlist->set( "hide", HideType::Audio );
}

void
MLTTrack::setVideoOutput( bool enabled )
{
    if ( enabled == true )
        if ( m_playlist->get_int( "hide" ) == HideType::VideoAndAudio )
            m_playlist->set( "hide", HideType::Audio );
        else
            m_playlist->set( "hide", HideType::None );
    else
        if ( m_playlist->get_int( "hide" ) == HideType::Audio )
            m_playlist->set( "hide", HideType::VideoAndAudio );
        else
            m_playlist->set( "hide", HideType::Video );
}
