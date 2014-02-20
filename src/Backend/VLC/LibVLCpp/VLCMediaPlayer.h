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

#include "vlc/vlc.h"

struct  libvlc_media_player_t;
struct  libvlc_event_t;
struct  libvlc_event_manager_t;

namespace   LibVLCpp
{
    class   Instance;
    class   Media;

    class   MediaPlayer : public Internal< libvlc_media_player_t >
    {
    public:

        typedef bool (*CheckEventCallback)(const MediaPlayer*, const libvlc_event_t*);

        MediaPlayer( Instance* vlcInstance );
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
        void                                setupVmemCallbacks( libvlc_video_lock_cb lock, libvlc_video_unlock_cb unlock,
                                                        libvlc_video_display_cb display, void* data );
        void                                setupVmem( const char* chroma, unsigned int width,
                                                       unsigned int height, unsigned int pitch );


        void                                configureWaitForEvent(const QList<int> &toWait,
                                                                    const QList<int> &cancel,
                                                                    CheckEventCallback callback = NULL );
        void                                configureWaitForEvent(int, const QList<int> &cancel,
                                                                    CheckEventCallback callback = NULL );

        void                                setName(const char *name );

        void                                registerEvents( libvlc_callback_t callback, void* data );
        void                                unregisterEvents( libvlc_callback_t callback, void* data );
        const char*                         eventName( libvlc_event_type_t event ) const;

    private:
        libvlc_event_manager_t*             p_em;
    };
}

#endif // VLCMEDIAPLAYER_H
