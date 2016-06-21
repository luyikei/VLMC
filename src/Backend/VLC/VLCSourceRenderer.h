/*****************************************************************************
 * VLCRenderer.h: Implementation of a backend using VLC
 *****************************************************************************
 * Copyright (C) 2008-2016 VideoLAN
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

#ifndef VLCRENDERER_H
#define VLCRENDERER_H

#include <QFlags>
#include <QString>
#include <QObject>

#include "vlcpp/vlc.hpp"

namespace Backend
{
class ISourceRendererEventCb
{
public:
    virtual ~ISourceRendererEventCb() = default;
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

namespace VLC
{

class RendererEventWatcher : public QObject, public Backend::ISourceRendererEventCb
{
    Q_OBJECT
public:
    explicit RendererEventWatcher(QObject *parent = 0) : QObject( parent ) {};

private:
    virtual void    onTimeChanged( int64_t time ) { emit timeChanged( time ); }
    virtual void    onPlaying() { emit playing(); }
    virtual void    onPaused() { emit paused(); }
    virtual void    onStopped() { emit stopped(); }
    virtual void    onEndReached() { emit endReached(); }
    virtual void    onVolumeChanged() { emit volumeChanged(); }
    virtual void    onPositionChanged( float pos ) { emit positionChanged( pos ); }
    virtual void    onLengthChanged( int64_t length ) { emit lengthChanged( length ); }
    virtual void    onErrorEncountered() { emit errorEncountered(); }

signals:
    void            timeChanged( qint64 );
    void            playing();
    void            paused();
    void            stopped();
    void            endReached();
    void            volumeChanged();
    void            positionChanged( float );
    void            lengthChanged( qint64 );
    void            errorEncountered();
};

class VLCBackend;
class VLCSource;
class VLCMemorySource;

class VLCSourceRenderer
{
public:

    enum Mode
    {
        Playback = 0,
        Imem = 1,
        VideoSmem = 1 << 1,
        AudioSmem = 1 << 2,
        FileOutput = 1 << 3
    };
    Q_DECLARE_FLAGS( Modes, Mode )

    typedef void (*VideoOutputLockCallback)( void* data, uint8_t** p_buffer, size_t size );
    typedef void (*VideoOutputUnlockCallback)( void* data, uint8_t* buffer, int width,
                                    int height, int bpp, size_t size, int64_t pts );
    typedef void (*AudioOutputLockCallback)( void* data, uint8_t** p_buffer, size_t size );
    typedef void (*AudioOutputUnlockCallback)( void* data, uint8_t* buffer, unsigned int channels,
                                               unsigned int rate, unsigned int nb_samples, unsigned int bits_per_sample,
                                               size_t size, int64_t pts );

    typedef int (*MemoryInputLockCallback)( void *data, const char* cookie, int64_t *dts, int64_t *pts,
                     uint32_t *flags, size_t *bufferSize, const void **buffer );
    typedef void (*MemoryInputUnlockCallback)( void *data, const char* cookie, size_t buffSize, void *buffer );

    VLCSourceRenderer( VLCBackend* backendInstance, VLCSource *source, ISourceRendererEventCb* callback );
    VLCSourceRenderer( VLCBackend* backendInstance, const VLCMemorySource *source, ISourceRendererEventCb* callback );
    virtual ~VLCSourceRenderer();

    virtual void    setName( const char* name );
    virtual void    start();
    virtual void    stop();
    virtual bool    isStopped();
    virtual void    playPause();
    virtual void    setPause( bool isPaused );
    virtual void    nextFrame();
    virtual void    previousFrame();
    virtual void    setOutputWidget( void *target );
    virtual int64_t time();
    virtual void    setTime( int64_t time );
    virtual void    setPosition( float position );
    virtual int     volume();
    virtual void    setVolume( int volume );

    virtual void    setOutputFile( const char* path );

    // Video output
    virtual void    setOutputVideoCodec( const char* fourCC );
    virtual void    setOutputWidth( unsigned int width );
    virtual void    setOutputHeight( unsigned int height );
    virtual void    setOutputFps( float fps );
    virtual void    setOutputVideoBitrate( unsigned int vBitrate );

    // Audio output:
    virtual void    setOutputAudioCodec( const char* fourCC );
    virtual void    setOutputAudioSampleRate( unsigned int sampleRate );
    virtual void    setOutputAudioNumberChannels( unsigned int nbChannels );
    virtual void    setOutputAudioBitrate( unsigned int aBitrate );

    // imem:
    virtual void    enableMemoryInput( void* data, MemoryInputLockCallback lockCallback, MemoryInputUnlockCallback unlockCallback );

    // smem:
    virtual void    enableOutputToMemory( void* videoData, void* audioData, VideoOutputLockCallback videoLock, VideoOutputUnlockCallback videoUnlock,
                                          AudioOutputLockCallback audioLock, AudioOutputUnlockCallback audioUnlock, bool timeSync );
    virtual void    enableVideoOutputToMemory( void* data, VideoOutputLockCallback lock, VideoOutputUnlockCallback unlock, bool timeSync );
    virtual void    enableAudioOutputToMemory( void* data, AudioOutputLockCallback lock,
                                               AudioOutputUnlockCallback unlock, bool timeSync );

    // Below is stuff which is not accessible through IRenderer
    void    setOption( const QString& option );

private:
    void            initMediaPlayer();
    void            setupStreamOutput();
    QString         setupFileOutput();

protected:
    VLCBackend*                 m_backend;
    QString                     m_name;
    Modes                       m_modes;
    // This is a copy, to avoid sharing all options across multiple renderers
    ::VLC::Media                m_media;
    ::VLC::MediaPlayer          m_mediaPlayer;
    ISourceRendererEventCb*     m_callback;
    QString                     m_outputFileName;
    QString                     m_smemChain;

    // Video output settings
    QString                     m_outputVideoFourCC;
    unsigned int                m_outputWidth;
    unsigned int                m_outputHeight;
    unsigned int                m_outputVideoBitrate;
    float                       m_outputFps;

    // Audio output settings
    unsigned int                m_outputAudioBitrate;
    unsigned int                m_outputNbChannels;
    unsigned int                m_outputSampleRate;
    QString                     m_outputAudioFourCC;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( VLCSourceRenderer::Modes );

} //VLC
} //Backend

#endif // VLCRENDERER_H
