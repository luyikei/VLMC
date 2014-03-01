/*****************************************************************************
 * VLCMediaPlayer.cpp: Binding for libvlc_media_player
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

#include "VLCMediaPlayer.h"
#include "VLCInstance.h"
#include "VLCMedia.h"

#include <QtGlobal>

using namespace LibVLCpp;

MediaPlayer::MediaPlayer( Instance* vlcInstance )
{
    m_internalPtr = libvlc_media_player_new( *vlcInstance );
    // Initialize the event manager
    p_em = libvlc_media_player_event_manager( m_internalPtr );
}

MediaPlayer::~MediaPlayer()
{

    stop();
    libvlc_media_player_release( m_internalPtr );
}

void
MediaPlayer::registerEvents( libvlc_callback_t callback, void* data )
{
    libvlc_event_attach( p_em, libvlc_MediaPlayerSnapshotTaken, callback, data );
    libvlc_event_attach( p_em, libvlc_MediaPlayerTimeChanged, callback, data );
    libvlc_event_attach( p_em, libvlc_MediaPlayerPlaying, callback, data );
    libvlc_event_attach( p_em, libvlc_MediaPlayerPaused, callback, data );
    libvlc_event_attach( p_em, libvlc_MediaPlayerStopped, callback, data );
    libvlc_event_attach( p_em, libvlc_MediaPlayerEndReached, callback, data );
    libvlc_event_attach( p_em, libvlc_MediaPlayerPositionChanged, callback, data );
    libvlc_event_attach( p_em, libvlc_MediaPlayerLengthChanged, callback, data );
    libvlc_event_attach( p_em, libvlc_MediaPlayerEncounteredError, callback, data );
    libvlc_event_attach( p_em, libvlc_MediaPlayerPausableChanged, callback, data );
    libvlc_event_attach( p_em, libvlc_MediaPlayerSeekableChanged, callback, data );
    libvlc_event_attach( p_em, libvlc_MediaPlayerVout, callback, data );
}

void
MediaPlayer::unregisterEvents( libvlc_callback_t callback, void *data )
{
    libvlc_event_detach( p_em, libvlc_MediaPlayerSnapshotTaken, callback, data );
    libvlc_event_detach( p_em, libvlc_MediaPlayerTimeChanged, callback, data );
    libvlc_event_detach( p_em, libvlc_MediaPlayerPlaying, callback, data );
    libvlc_event_detach( p_em, libvlc_MediaPlayerPaused, callback, data );
    libvlc_event_detach( p_em, libvlc_MediaPlayerStopped, callback, data );
    libvlc_event_detach( p_em, libvlc_MediaPlayerEndReached, callback, data );
    libvlc_event_detach( p_em, libvlc_MediaPlayerPositionChanged, callback, data );
    libvlc_event_detach( p_em, libvlc_MediaPlayerLengthChanged, callback, data );
    libvlc_event_detach( p_em, libvlc_MediaPlayerEncounteredError, callback, data );
    libvlc_event_detach( p_em, libvlc_MediaPlayerPausableChanged, callback, data );
    libvlc_event_detach( p_em, libvlc_MediaPlayerSeekableChanged, callback, data );
    libvlc_event_detach( p_em, libvlc_MediaPlayerVout, callback, data );
}

const char*
MediaPlayer::eventName(libvlc_event_type_t event) const
{
    return libvlc_event_type_name( event );
}

void
MediaPlayer::play()
{
    //vlmcDebug() << "Asking for play media player";
    libvlc_media_player_play( m_internalPtr );
}

void
MediaPlayer::pause()
{
    libvlc_media_player_pause( m_internalPtr );
}

void
MediaPlayer::stop()
{
    //vlmcDebug() << "Asking for stop media player";
    libvlc_media_player_stop( m_internalPtr );
}

int
MediaPlayer::getVolume()
{
    return libvlc_audio_get_volume( m_internalPtr );
}

int
MediaPlayer::setVolume( int volume )
{
    //Returns 0 if the volume was set, -1 if it was out of range
    return libvlc_audio_set_volume( m_internalPtr, volume );
}

int64_t
MediaPlayer::getTime()
{
    return libvlc_media_player_get_time( m_internalPtr );
}

void
MediaPlayer::setTime( int64_t time )
{
    libvlc_media_player_set_time( m_internalPtr, time );
}

float
MediaPlayer::getPosition()
{
    return libvlc_media_player_get_position( m_internalPtr );
}

void
MediaPlayer::setPosition( float pos )
{
    libvlc_media_player_set_position( m_internalPtr, pos );
}

int64_t
MediaPlayer::getLength()
{
    return libvlc_media_player_get_length( m_internalPtr );
}

void
MediaPlayer::takeSnapshot( const char* outputFile, unsigned int width, unsigned int heigth )
{
    libvlc_video_take_snapshot( *this, 0, outputFile, width, heigth );
}

bool
MediaPlayer::isPlaying()
{
    return ( libvlc_media_player_is_playing( m_internalPtr ) == 1 );
}

bool
MediaPlayer::isSeekable()
{
    return ( libvlc_media_player_is_seekable( m_internalPtr ) == 1 );
}

void
MediaPlayer::setDrawable( void* drawable )
{
#if defined ( Q_OS_MAC )
    libvlc_media_player_set_nsobject( m_internalPtr, drawable );
#elif defined ( Q_OS_UNIX )
    libvlc_media_player_set_xwindow( m_internalPtr, reinterpret_cast<intptr_t>( drawable ) );
#elif defined ( Q_OS_WIN )
    libvlc_media_player_set_hwnd( m_internalPtr, drawable );
#endif
}

void
MediaPlayer::setMedia( Media* media )
{
    libvlc_media_player_set_media( m_internalPtr, media->getInternalPtr() );
}

void
MediaPlayer::getSize( unsigned int *outWidth, unsigned int *outHeight )
{
    libvlc_video_get_size( m_internalPtr, 0, outWidth, outHeight );
}

float
MediaPlayer::getFps()
{
    return libvlc_media_player_get_fps( m_internalPtr );
}

void
MediaPlayer::nextFrame()
{
    libvlc_media_player_next_frame( m_internalPtr );
}

bool
MediaPlayer::hasVout()
{
    return libvlc_media_player_has_vout( m_internalPtr ) > 0;
}

int
MediaPlayer::getNbAudioTrack()
{
    return libvlc_audio_get_track_count( m_internalPtr );
}

int
MediaPlayer::getNbVideoTrack()
{
    return libvlc_video_get_track_count( m_internalPtr );
}

void
MediaPlayer::setKeyInput( bool enabled )
{
    libvlc_video_set_key_input( m_internalPtr, enabled );
}

void
MediaPlayer::setAudioOutput(const char *module)
{
    libvlc_audio_output_set( m_internalPtr, module );
}

void
MediaPlayer::disableTitle()
{
    libvlc_media_player_set_video_title_display( *this, libvlc_position_disable, 0 );
}

void
MediaPlayer::setupVmemCallbacks(libvlc_video_lock_cb lock, libvlc_video_unlock_cb unlock, libvlc_video_display_cb display, void *data)
{
    libvlc_video_set_callbacks( *this, lock, unlock, display, data );
}

void
MediaPlayer::setupVmem(const char *chroma, unsigned int width, unsigned int height, unsigned int pitch)
{
    libvlc_video_set_format( *this, chroma, width, height, pitch );
}


bool
LibVLCpp::MediaPlayer::willPlay()
{
    return libvlc_media_player_will_play( *this ) != 0;
}
