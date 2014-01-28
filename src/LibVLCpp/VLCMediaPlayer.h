/*****************************************************************************
 * VLCMediaPlayer.h: Binding for libvlc_media_player
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

#ifndef VLCMEDIAPLAYER_H
#define VLCMEDIAPLAYER_H

#include <QList>
#include <QMutex>
#include <QObject>
#include <QString>
#include <QWaitCondition>

#include "VLCpp.hpp"

struct  libvlc_media_player_t;
struct  libvlc_event_t;
struct  libvlc_event_manager_t;

namespace   LibVLCpp
{
    class   Media;
    class   MediaPlayer : public QObject, public Internal< libvlc_media_player_t >
    {
        Q_OBJECT
    public:
        enum    EventWaitResult
        {
            Success, ///The event has been emited.
            Canceled,///A cancelation event has been emited first.
            Timeout  ///Timeout has been reached.
        };
        typedef bool (*CheckEventCallback)(const MediaPlayer*, const libvlc_event_t*);

        MediaPlayer();
        MediaPlayer( Media* media );
        ~MediaPlayer();
        void                                play();
        void                                pause();
        void                                stop();
        int                                 getVolume();
        int                                 setVolume( int volume );
        qint64                              getTime();
        void                                setTime( qint64 time );
        float                               getPosition();
        void                                setPosition( float pos );
        /**
         \return        The length, in milliseconds.
        */
        qint64                              getLength();
        void                                takeSnapshot( const char* outputFile, unsigned int width, unsigned int heigth );
        bool                                isPlaying();
        bool                                isSeekable();
        void                                setDrawable( void* drawable );
        void                                setMedia(Media* media);
        void                                getSize( quint32 *outWidth, quint32 *outHeight);
        float                               getFps();
        void                                nextFrame();
        bool                                hasVout();
        const QString&                      getLoadedFileName() const;
        QString                             getLoadedMRL();
        int                                 getNbAudioTrack();
        int                                 getNbVideoTrack();
        void                                setKeyInput( bool enabled );
        void                                setAudioOutput(const char* module);
        void                                disableTitle();

        /**
         * @brief configure the usage of waitForEvent.
         *
         * This method MUST be called before play(). It will lock a mutex
         * that only will be unlocked in the waitForEvent() method. This mutex prevent
         * any event processing.
         * Using this method in any other scheme than:
         * \li mediaPlayer->configureWaitForEvent(...);
         * \li mediaPlayer->play();
         * \li mediaPlayer->waitForEvents(...);
         *
         * is likely to result in a deadlock.
         *
         * @param toWait The list of events to wait for.
         * @param cancel A list of events that would cancel the waiting process.
         * @param callback A callback that will be called to check if this event is
         *                  ok (for instance, accoding to the value of a changed variable)
         *                  Callback will be called from an external thread.
         */
        void                                configureWaitForEvent(const QList<int> &toWait,
                                                                    const QList<int> &cancel,
                                                                    CheckEventCallback callback = NULL );
        void                                configureWaitForEvent(int, const QList<int> &cancel,
                                                                    CheckEventCallback callback = NULL );
        /**
         * @brief This method will wait for one of the events specified by the
         * list given to configureWaitForEvent().
         *
         * In case the waiting process should be canceled by
         * some specific events, they shall be passed in the cancel vector.
         * A timeout parameter can be passed. Default is to wait forever.
         *
         * This method MUST be called IMMEDIATLY AFTER play(), as a mutex is held,
         * and would block any event processing until the actual waiting started.
         *
         * This method is concieved to wait on one unique given set of events.
         * You can't wait for another set of events from another thread at the same time.
         *
         * Events (regardless of their presence in the wait or cancel list) will
         * still be propagated to every potential receiver.
         *
         * @param timeout The maximum amount of time (in ms) to wait for events.
         *
         * @warning This method WILL BLOCK and therefore should NEVER been called
         *          from either a VLC thread, or Qt's main thread.
         *
         * @returns A value as defined in the EventWaitResult enum.
         */
        MediaPlayer::EventWaitResult        waitForEvent( unsigned long timeout = ULONG_MAX );

    private:
        static void                         callbacks( const libvlc_event_t* event, void* self );
        void                                registerEvents();
        void                                checkForWaitedEvents( const libvlc_event_t* event );

    private:
        libvlc_event_manager_t*             p_em;
        Media*                              m_media;

        QWaitCondition                      m_waitCond;
        QMutex                              m_mutex;
        int                                 m_eventReceived;
        CheckEventCallback                  m_eventsCallback;
        QList<int>                          m_eventsExpected;
        QList<int>                          m_eventsCancel;

    signals:
        void                                snapshotTaken( const char* );
        void                                timeChanged( qint64 );
        void                                playing();
        void                                paused();
        void                                stopped();
        void                                endReached();
        void                                volumeChanged();
        void                                positionChanged( float );
        void                                lengthChanged( qint64 );
        void                                errorEncountered();
    };
}

#endif // VLCMEDIAPLAYER_H
