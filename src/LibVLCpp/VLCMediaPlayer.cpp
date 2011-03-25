/*****************************************************************************
 * VLCMediaPlayer.cpp: Binding for libvlc_media_player
 *****************************************************************************
 * Copyright (C) 2008-2010 VideoLAN
 *
 * Authors: Hugo Beauzée-Luyssen <beauze.h@gmail.com>
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

#include <QtGlobal>
#include <QtDebug>
#include <cassert>
#include "VLCMediaPlayer.h"
#include "VLCInstance.h"
#include "VLCMedia.h"

using namespace LibVLCpp;

MediaPlayer::MediaPlayer() : m_media( NULL )
{
    m_internalPtr = libvlc_media_player_new( LibVLCpp::Instance::getInstance()->getInternalPtr() );
    // Initialize the event manager
    p_em = libvlc_media_player_event_manager( m_internalPtr );
    registerEvents();
}

MediaPlayer::MediaPlayer( Media* media ) : m_media( media )
{
    m_internalPtr = libvlc_media_player_new_from_media( media->getInternalPtr() );

    // Initialize the event manager
    p_em = libvlc_media_player_event_manager( m_internalPtr );
    registerEvents();
}

MediaPlayer::~MediaPlayer()
{
    libvlc_event_detach( p_em, libvlc_MediaPlayerSnapshotTaken, callbacks, this );
    libvlc_event_detach( p_em, libvlc_MediaPlayerTimeChanged, callbacks, this );
    libvlc_event_detach( p_em, libvlc_MediaPlayerPlaying, callbacks, this );
    libvlc_event_detach( p_em, libvlc_MediaPlayerPaused, callbacks, this );
    libvlc_event_detach( p_em, libvlc_MediaPlayerStopped, callbacks, this );
    libvlc_event_detach( p_em, libvlc_MediaPlayerEndReached, callbacks, this );
    libvlc_event_detach( p_em, libvlc_MediaPlayerPositionChanged, callbacks, this );
    stop();
    libvlc_media_player_release( m_internalPtr );
}

void
MediaPlayer::registerEvents()
{
    // Register the callback
    libvlc_event_attach( p_em, libvlc_MediaPlayerSnapshotTaken,   callbacks, this );
    libvlc_event_attach( p_em, libvlc_MediaPlayerTimeChanged,     callbacks, this );
    libvlc_event_attach( p_em, libvlc_MediaPlayerPlaying,         callbacks, this );
    libvlc_event_attach( p_em, libvlc_MediaPlayerPaused,          callbacks, this );
    libvlc_event_attach( p_em, libvlc_MediaPlayerStopped,         callbacks, this );
    libvlc_event_attach( p_em, libvlc_MediaPlayerEndReached,      callbacks, this );
    libvlc_event_attach( p_em, libvlc_MediaPlayerPositionChanged, callbacks, this );
    libvlc_event_attach( p_em, libvlc_MediaPlayerLengthChanged,   callbacks, this );
    libvlc_event_attach( p_em, libvlc_MediaPlayerEncounteredError,callbacks, this );
    libvlc_event_attach( p_em, libvlc_MediaPlayerPausableChanged, callbacks, this );
    libvlc_event_attach( p_em, libvlc_MediaPlayerSeekableChanged, callbacks, this );
}

/**
 * Event dispatcher.
 */
void
MediaPlayer::callbacks( const libvlc_event_t* event, void* ptr )
{
    MediaPlayer* self = reinterpret_cast<MediaPlayer*>( ptr );
    switch ( event->type )
    {
    case libvlc_MediaPlayerPlaying:
        //qDebug() << "Media player playing";
        self->emit playing();
        break;
    case libvlc_MediaPlayerPaused:
        //qDebug() << "Media player paused";
        self->emit paused();
        break;
    case libvlc_MediaPlayerStopped:
        //qDebug() << "Media player stopped";
        self->emit stopped();
        break;
    case libvlc_MediaPlayerEndReached:
        //qDebug() << "Media player end reached";
        self->emit endReached();
        break;
    case libvlc_MediaPlayerTimeChanged:
        self->emit timeChanged( event->u.media_player_time_changed.new_time );
        break;
    case libvlc_MediaPlayerPositionChanged:
        //qDebug() << self << "position changed : " << event->u.media_player_position_changed.new_position;
        self->emit positionChanged( event->u.media_player_position_changed.new_position );
        break;
    case libvlc_MediaPlayerLengthChanged:
        self->emit lengthChanged( event->u.media_player_length_changed.new_length );
        break;
    case libvlc_MediaPlayerSnapshotTaken:
        self->emit snapshotTaken( event->u.media_player_snapshot_taken.psz_filename );
        break;
    case libvlc_MediaPlayerEncounteredError:
        qDebug() << '[' << (void*)self << "] libvlc_MediaPlayerEncounteredError received."
                << "This is not looking good...";
        self->emit errorEncountered();
        break;
    case libvlc_MediaPlayerSeekableChanged:
        // TODO: Later change it to an event that corresponds volume change, when this thing gets fixed in libvlc
        self->emit volumeChanged();
        break;
    case libvlc_MediaPlayerPausableChanged:
    case libvlc_MediaPlayerTitleChanged:
    case libvlc_MediaPlayerNothingSpecial:
    case libvlc_MediaPlayerOpening:
    case libvlc_MediaPlayerBuffering:
    case libvlc_MediaPlayerForward:
    case libvlc_MediaPlayerBackward:
    default:
//        qDebug() << "Unknown mediaPlayerEvent: " << event->type;
        break;
    }
}

void
MediaPlayer::play()
{
    //qDebug() << "Asking for play media player";
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
    //qDebug() << "Asking for stop media player";
    libvlc_media_player_stop( m_internalPtr );
}

int
MediaPlayer::getVolume()
{
    int volume = libvlc_audio_get_volume( m_internalPtr );
    return volume;
}

int
MediaPlayer::setVolume( int volume )
{
    //Returns 0 if the volume was set, -1 if it was out of range
    return libvlc_audio_set_volume( m_internalPtr, volume );
}

qint64
MediaPlayer::getTime()
{
    qint64 t = libvlc_media_player_get_time( m_internalPtr );
    return t;
}

void
MediaPlayer::setTime( qint64 time )
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

qint64
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
#if defined ( Q_WS_MAC )
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
MediaPlayer::getSize( quint32 *outWidth, quint32 *outHeight )
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
    return libvlc_media_player_has_vout( m_internalPtr );
}

const QString&
MediaPlayer::getLoadedFileName() const
{
    return m_media->getFileName();
}

QString
MediaPlayer::getLoadedMRL()
{
    Media::internalPtr     media = libvlc_media_player_get_media( m_internalPtr );
    char* str = libvlc_media_get_mrl( media );
    return QString( str );
}

int
MediaPlayer::getNbAudioTrack()
{
    int res = libvlc_audio_get_track_count( m_internalPtr );
    return res;
}

int
MediaPlayer::getNbVideoTrack()
{
    int res = libvlc_video_get_track_count( m_internalPtr );
    return res;
}

void
MediaPlayer::setKeyInput( bool enabled )
{
    libvlc_video_set_key_input( m_internalPtr, enabled );
}
