/*****************************************************************************
 * VLCRenderer.h: Implementation of a backend using VLC
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

#ifndef VLCRENDERER_H
#define VLCRENDERER_H

#include <QFlags>
#include <QString>

#include "Backend/ISourceRenderer.h"

#include "VLCMediaPlayer.h"

namespace Backend
{
namespace VLC
{

class VLCBackend;
class VLCSource;
class VLCMemorySource;

class VLCSourceRenderer : public ISourceRenderer
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

    VLCSourceRenderer( VLCBackend* backendInstance, VLCSource *source, ISourceRendererEventCb* callback );
    VLCSourceRenderer( VLCBackend* backendInstance, const VLCMemorySource *source, ISourceRendererEventCb* callback );
    virtual ~VLCSourceRenderer();

    virtual void    setName( const char* name );
    virtual void    start();
    virtual void    stop();
    virtual bool    isStopped() override;
    virtual void    playPause();
    virtual void    setPause( bool isPaused ) override;
    virtual void    nextFrame();
    virtual void    previousFrame();
    virtual void    setOutputWidget( void *target );
    virtual int64_t time() const;
    virtual void    setTime( int64_t time );
    virtual void    setPosition( float position );
    virtual int     volume() const;
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
    virtual void    enableVideoOutputToMemory( void* data, VideoOutputLockCallback lock, VideoOutputUnlockCallback unlock, bool timeSync );
    virtual void    enableAudioOutputToMemory( void* data, AudioOutputLockCallback lock,
                                               AudioOutputUnlockCallback unlock, bool timeSync );

    // Below is stuff which is not accessible through IRenderer
    virtual void    setOption( const QString& option );

private:
    void            initMediaPlayer();
    void            setupStreamOutput();
    QString         setupFileOutput();
    static void     eventsCallback( const libvlc_event_t* event, void* data );

protected:
    VLCBackend*                 m_backend;
    QString                     m_name;
    Modes                       m_modes;
    // This is a copy, to avoid sharing all options across multiple renderers
    LibVLCpp::Media*            m_media;
    LibVLCpp::MediaPlayer*      m_mediaPlayer;
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
