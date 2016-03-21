/*****************************************************************************
 * ISourceRenderer: Defines an interface to render a frame from a media source
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

#ifndef IRENDERER_H
#define IRENDERER_H

#include <stdint.h>

namespace Backend
{
    class ISourceRendererEventCb
    {
    public:
        virtual ~ISourceRendererEventCb() {}
        virtual void    onTimeChanged( int64_t ) = 0;
        virtual void    onPlaying() = 0;
        virtual void    onPaused() = 0;
        virtual void    onStopped() = 0;
        virtual void    onEndReached() = 0;
        virtual void    onVolumeChanged() = 0;
        virtual void    onPositionChanged( float ) = 0;
        virtual void    onLengthChanged( int64_t ) = 0;
        virtual void    onErrorEncountered() = 0;
    };

    class ISourceRenderer
    {
    public:
        virtual ~ISourceRenderer() {}

        typedef void (*VideoOutputLockCallback)( void* data, uint8_t** p_buffer, size_t size );
        typedef void (*VideoOutputUnlockCallback)( void* data, uint8_t* buffer, int width,
                                        int height, int bpp, size_t size, int64_t pts );
        typedef void (*AudioOutputLockCallback)( void* data, uint8_t** p_buffer, size_t size );
        typedef void (*AudioOutputUnlockCallback)( void* data, uint8_t* buffer, unsigned int channels,
                                                   unsigned int rate, unsigned int nb_samples, unsigned int bits_per_sample,
                                                   size_t size, int64_t pts );

        typedef int (*MemoryInputLockCallback)( void *data, const char* cookie, int64_t *dts, int64_t *pts,
                         quint32 *flags, size_t *bufferSize, const void **buffer );
        typedef void (*MemoryInputUnlockCallback)( void *data, const char* cookie, size_t buffSize, void *buffer );

        virtual void    setName( const char* name ) = 0;
        /**
         * @brief start Initializes and launches playback.
         */
        virtual void    start() = 0;
        virtual void    stop() = 0;
        virtual bool    isStopped() = 0;
        virtual void    playPause() = 0;
        virtual void    setPause( bool isPaused ) = 0;
        virtual void    nextFrame() = 0;
        virtual void    previousFrame() = 0;

        virtual int     volume() = 0;
        virtual void    setVolume( int volume ) = 0;
        /**
         * @brief setOutputWidget   Will direct the rendering to the specified widget.
         *
         * @param target            The widget to render on. This parameter is platform dependent.
         */
        virtual void    setOutputWidget( void* target ) = 0;

        // Video output
        virtual void    setOutputFile( const char* path ) = 0;
        virtual void    setOutputVideoCodec( const char* fourCC ) = 0;
        virtual void    setOutputWidth( unsigned int width ) = 0;
        virtual void    setOutputHeight( unsigned int height ) = 0;
        virtual void    setOutputFps( float fps ) = 0;
        virtual void    setOutputVideoBitrate( unsigned int vBitrate ) = 0;

        // Audio output
        virtual void    setOutputAudioCodec( const char* fourCC ) = 0;
        virtual void    setOutputAudioSampleRate( unsigned int sampleRate ) = 0;
        virtual void    setOutputAudioNumberChannels( unsigned int nbChannels ) = 0;
        virtual void    setOutputAudioBitrate( unsigned int aBitrate ) = 0;

        virtual int64_t time() = 0;
        virtual void    setTime( int64_t time ) = 0;
        virtual void    setPosition( float position ) = 0;

        // For video output to memory:
        virtual void enableVideoOutputToMemory( void* data, VideoOutputLockCallback lock, VideoOutputUnlockCallback unlock, bool timeSync ) = 0;
        // For audio output to memory:
        virtual void enableAudioOutputToMemory( void* data, AudioOutputLockCallback lock, AudioOutputUnlockCallback unlock, bool timeSync ) = 0;

        // For memory input:
        virtual void enableMemoryInput( void* data, MemoryInputLockCallback lockCallback,
                                        MemoryInputUnlockCallback unlockCallback ) = 0;
    };
}

#endif // IRENDERER_H
