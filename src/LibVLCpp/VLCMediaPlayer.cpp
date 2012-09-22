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

#define DETACH(event) libvlc_event_detach( p_em, event, callbacks, this );

MediaPlayer::~MediaPlayer()
{
    DETACH( libvlc_MediaPlayerSnapshotTaken );
    DETACH( libvlc_MediaPlayerTimeChanged );
    DETACH( libvlc_MediaPlayerPlaying );
    DETACH( libvlc_MediaPlayerPaused );
    DETACH( libvlc_MediaPlayerStopped );
    DETACH( libvlc_MediaPlayerEndReached );
    DETACH( libvlc_MediaPlayerPositionChanged );
    DETACH( libvlc_MediaPlayerLengthChanged );
    DETACH( libvlc_MediaPlayerEncounteredError );
    DETACH( libvlc_MediaPlayerPausableChanged );
    DETACH( libvlc_MediaPlayerSeekableChanged );
    DETACH( libvlc_MediaPlayerVout );
    stop();
    libvlc_media_player_release( m_internalPtr );
}

#undef DETACH
#define ATTACH(event) libvlc_event_attach( p_em, event, callbacks, this );

void
MediaPlayer::registerEvents()
{
    // Register the callback
    ATTACH( libvlc_MediaPlayerSnapshotTaken );
    ATTACH( libvlc_MediaPlayerTimeChanged );
    ATTACH( libvlc_MediaPlayerPlaying );
    ATTACH( libvlc_MediaPlayerPaused );
    ATTACH( libvlc_MediaPlayerStopped );
    ATTACH( libvlc_MediaPlayerEndReached );
    ATTACH( libvlc_MediaPlayerPositionChanged );
    ATTACH( libvlc_MediaPlayerLengthChanged );
    ATTACH( libvlc_MediaPlayerEncounteredError );
    ATTACH( libvlc_MediaPlayerPausableChanged );
    ATTACH( libvlc_MediaPlayerSeekableChanged );
    ATTACH( libvlc_MediaPlayerVout );
}

#undef ATTACH

/**
 * Event dispatcher.
 */
void
MediaPlayer::checkForWaitedEvents(const libvlc_event_t *event)
{
    QMutexLocker lock( &m_mutex );

    // Use the user provided callback to check if this event suits him.
    // This is intented to filter out some events, such as multiple length changed
    // with a value of 0
    if ( m_eventsCallback != NULL && m_eventsCallback( this, event ) == false )
        return ;
    if ( m_eventsExpected.contains( event->type ) == true )
    {
        m_eventReceived = event->type;
        m_waitCond.wakeAll();
    }
    else if ( m_eventsCancel.contains( event->type ) == true )
    {
        m_eventReceived = event->type;
        m_waitCond.wakeAll();
    }
    //Otherwise this is an event we don't care about.
}

void
MediaPlayer::callbacks( const libvlc_event_t* event, void* ptr )
{
    Q_ASSERT_X( event->type >= libvlc_MediaPlayerMediaChanged &&
                event->type < libvlc_MediaListItemAdded, "event callback", "Only libvlc_MediaPlayer* events are supported" );

    MediaPlayer* self = reinterpret_cast<MediaPlayer*>( ptr );

    self->checkForWaitedEvents( event );

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

void MediaPlayer::setAudioOutput(const char *module)
{
    libvlc_audio_output_set( m_internalPtr, module );
}

void
MediaPlayer::configureWaitForEvent( const QList<int> &toWait, const QList<int> &cancel,
                                    CheckEventCallback callback )
{
    //This mutex will only be unlocked when entering the wait condition, and upon
    //wait completion.
    m_mutex.lock();
    Q_ASSERT_X( m_eventsExpected.size() == 0 && m_eventsCancel.size() == 0,
               "waitForEvent", "waitForEvent is not supposed to be used simultaneously" );
    m_eventsExpected.append( toWait );
    m_eventsCancel.append( cancel );
    m_eventReceived = 0;
    m_eventsCallback = callback;
}

void
MediaPlayer::configureWaitForEvent(int toWait, const QList<int> &cancel, MediaPlayer::CheckEventCallback callback)
{
    QList<int> toWaitList;
    toWaitList.append( toWait );
    configureWaitForEvent( toWaitList, cancel, callback );
}

MediaPlayer::EventWaitResult
MediaPlayer::waitForEvent( unsigned long timeoutDuration )
{
    bool timeout = !m_waitCond.wait( &m_mutex, timeoutDuration );
    //m_mutex is now locked.
    bool found = ( timeout == false && m_eventsExpected.contains( m_eventReceived ) == true );
    m_eventsCancel.clear();
    m_eventsExpected.clear();
    m_eventsCallback = NULL;
    m_eventReceived = 0;
    m_mutex.unlock();
    //And give feedback:
    if ( timeout == true )
        return Timeout;
    return ( found ? Success : Canceled );
}
