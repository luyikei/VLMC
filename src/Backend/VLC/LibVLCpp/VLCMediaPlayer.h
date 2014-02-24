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
        int64_t                             getTime();
        void                                setTime(int64_t time );
        float                               getPosition();
        void                                setPosition( float pos );
        /**
         \return        The length, in milliseconds.
        */
        int64_t getLength();
        void                                takeSnapshot( const char* outputFile, unsigned int width, unsigned int heigth );
        bool                                isPlaying();
        bool                                isSeekable();
        void                                setDrawable( void* drawable );
        void                                setMedia(Media* media);
        void                                getSize( unsigned int *outWidth, unsigned int *outHeight);
        float                               getFps();
        void                                nextFrame();
        bool                                hasVout();
        int                                 getNbAudioTrack();
        int                                 getNbVideoTrack();
        void                                setKeyInput( bool enabled );
        void                                setAudioOutput(const char* module);
        void                                disableTitle();
        void                                setupVmemCallbacks( libvlc_video_lock_cb lock, libvlc_video_unlock_cb unlock,
                                                        libvlc_video_display_cb display, void* data );
        void                                setupVmem( const char* chroma, unsigned int width,
                                                       unsigned int height, unsigned int pitch );


        void                                registerEvents( libvlc_callback_t callback, void* data );
        void                                unregisterEvents( libvlc_callback_t callback, void* data );
        const char*                         eventName( libvlc_event_type_t event ) const;

    private:
        libvlc_event_manager_t*             p_em;
    };
}

#endif // VLCMEDIAPLAYER_H
