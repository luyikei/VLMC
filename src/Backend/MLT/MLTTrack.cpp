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
    : MLTInput()
{
    MLTProfile& mltProfile = static_cast<MLTProfile&>( profile );
    m_playlist = new Mlt::Playlist( *mltProfile.m_profile );
}

MLTTrack::~MLTTrack()
{
    delete m_playlist;
}

Mlt::Playlist*
MLTTrack::playlist()
{
    return m_playlist;
}

Mlt::Playlist*
MLTTrack::playlist() const
{
    return m_playlist;
}

Mlt::Producer*
MLTTrack::producer()
{
    return playlist();
}

Mlt::Producer*
MLTTrack::producer() const
{
    return playlist();
}

bool
MLTTrack::insertAt( Backend::IInput& input, int64_t startFrame )
{
    auto mltInput = dynamic_cast<MLTInput*>( &input );
    assert( mltInput );
    return !playlist()->insert_at( (int)startFrame, mltInput->producer(), 1 );
}

bool
MLTTrack::append( Backend::IInput& input )
{
    auto mltInput = dynamic_cast<MLTInput*>( &input );
    assert( mltInput );
    return !playlist()->append( *mltInput->producer() );
}

bool
MLTTrack::remove( int index )
{
    auto ret = playlist()->remove( index );

    // Remove last blanks.
    for ( int i = playlist()->count() - 1; i >= 0; --i )
    {
        if ( strcmp( playlist()->clip_info( i )->resource, "blank" ) == 0 )
            playlist()->remove( i );
        else
            break;
    }

    return !ret;
}

bool
MLTTrack::move( int src, int dist )
{
    return !playlist()->move( src, dist );
}

Backend::IInput*
MLTTrack::clip( int index ) const
{
    return new MLTInput( playlist()->get_clip( index ) );
}

Backend::IInput*
MLTTrack::clipAt( int64_t position ) const
{
    return new MLTInput( playlist()->get_clip_at( (int)position ) );
}

bool
MLTTrack::resizeClip( int clip, int64_t begin, int64_t end )
{
    return !playlist()->resize_clip( clip, (int)begin, (int)end );
}

int
MLTTrack::clipIndexAt( int64_t position )
{
    return playlist()->get_clip_index_at( (int)position );
}

int
MLTTrack::count() const
{
    return playlist()->count();
}

void
MLTTrack::clear()
{
    playlist()->clear();
}

void
MLTTrack::setMute( bool muted )
{
    if ( muted == false )
        if ( playlist()->get_int( "hide" ) == HideType::VideoAndAudio )
            playlist()->set( "hide", HideType::Video );
        else
            playlist()->set( "hide", HideType::None );
    else
        if ( playlist()->get_int( "hide" ) == HideType::Video )
            playlist()->set( "hide", HideType::VideoAndAudio );
        else
            playlist()->set( "hide", HideType::Audio );
}

void
MLTTrack::setVideoEnabled( bool enabled )
{
    if ( enabled == true )
        if ( playlist()->get_int( "hide" ) == HideType::VideoAndAudio )
            playlist()->set( "hide", HideType::Audio );
        else
            playlist()->set( "hide", HideType::None );
    else
        if ( playlist()->get_int( "hide" ) == HideType::Audio )
            playlist()->set( "hide", HideType::VideoAndAudio );
        else
            playlist()->set( "hide", HideType::Video );
}
