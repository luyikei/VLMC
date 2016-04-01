/*****************************************************************************
 * VLCSourceRenderer.cpp: Implementation of a backend using VLC
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

#include <cstdio>

#include <QString>
#include <QStringBuilder>

#include "VLCBackend.h"
#include "VLCSourceRenderer.h"
#include "VLCSource.h"
#include "VLCMemorySource.h"
#include "Tools/VlmcDebug.h"

using namespace Backend;
using namespace Backend::VLC;

VLCSourceRenderer::VLCSourceRenderer( VLCBackend* backendInstance, VLCSource *source, ISourceRendererEventCb *callback )
    : m_backend( backendInstance )
    , m_modes( Playback )
    , m_callback( callback )
    , m_outputWidth( 0 )
    , m_outputHeight( 0 )
    , m_outputVideoBitrate( 0 )
    , m_outputFps( .0f )
    , m_outputAudioBitrate( 0 )
{
    m_media = ::VLC::Media( backendInstance->vlcInstance(), source->media().mrl(), ::VLC::Media::FromLocation );
    initMediaPlayer();
}

VLCSourceRenderer::VLCSourceRenderer( VLCBackend* backendInstance, const VLCMemorySource *source, ISourceRendererEventCb *callback )
    : m_backend( backendInstance )
    , m_callback( callback )
{
    char        videoString[512];
    char        inputSlave[256];
    char        audioParameters[256];

    sprintf( videoString, "imem://width=%i:height=%i:dar=%s:fps=%f/1:cookie=0:codec=%s:cat=2:caching=0",
             source->width(), source->height(), qPrintable( source->aspectRatio() ), source->fps(), "RV32" );
    sprintf( audioParameters, "cookie=1:cat=1:codec=f32l:samplerate=%u:channels=%u:caching=0",
                source->sampleRate(), source->numberChannels() );
    strcpy( inputSlave, ":input-slave=imem://" );
    strcat( inputSlave, audioParameters );

    m_media = ::VLC::Media( backendInstance->vlcInstance(), videoString, ::VLC::Media::FromLocation );
    setOption( inputSlave );
    initMediaPlayer();
}

void
VLCSourceRenderer::initMediaPlayer()
{
    m_mediaPlayer = ::VLC::MediaPlayer( m_backend->vlcInstance() );
    if ( m_callback != nullptr )
    {
        auto& em = m_mediaPlayer.eventManager();
        em.onPlaying( [this]() { m_callback->onPlaying(); } );
        em.onPaused( [this]() { m_callback->onPaused(); } );
        em.onStopped( [this]() { m_callback->onStopped(); } );
        em.onEndReached( [this]() { m_callback->onEndReached(); } );
        em.onTimeChanged( [this]( int64_t t ) { m_callback->onTimeChanged( t ); } );
        em.onPositionChanged( [this]( float p ) { m_callback->onPositionChanged( p ); } );
        em.onLengthChanged( [this]( int64_t l ) { m_callback->onLengthChanged( l ); } );
        em.onEncounteredError( [this](){ m_callback->onErrorEncountered(); } );
        // TODO: Later change it to an event that corresponds volume change, when this thing gets fixed in libvlc
        em.onSeekableChanged( [this]( bool ) { m_callback->onVolumeChanged(); } );
    }
    m_mediaPlayer.setKeyInput( false );
    m_mediaPlayer.setVideoTitleDisplay( libvlc_position_disable, 0 );
}

VLCSourceRenderer::~VLCSourceRenderer()
{
    stop();
}

void
VLCSourceRenderer::setName(const char *name)
{
    m_name = name;
}

void
VLCSourceRenderer::setupStreamOutput()
{
    // In case of pure imem (ie project preview, which doesn't involve transcode)
    // or simple clip playback, we're not interested in setting up a stream-output chain
    if ( m_modes == Playback || m_modes == Imem )
        return ;

    Q_ASSERT( ( m_modes.testFlag( VideoSmem ) == false && m_modes.testFlag( AudioSmem ) == false ) ||
              m_smemChain.isEmpty() == false );

    QString     transcodeStr = ":sout=#transcode{";
    if ( m_modes.testFlag( VideoSmem ) || m_modes.testFlag( FileOutput ) )
    {
        Q_ASSERT( m_modes.testFlag( AudioSmem ) == false );

        if ( m_outputVideoFourCC.isNull() == false )
            transcodeStr += ",vcodec=" + m_outputVideoFourCC;
        if ( m_outputVideoBitrate > 0 )
            transcodeStr += ",vb=" + QString::number( m_outputVideoBitrate );
        if ( m_outputFps > 0.1f )
            transcodeStr += ",fps=" + QString::number( m_outputFps );
        if ( m_outputWidth > 0 )
            transcodeStr += ",width=" + QString::number( m_outputWidth );
        if ( m_outputHeight > 0 )
            transcodeStr += ",height=" + QString::number( m_outputHeight );
    }
    if ( m_modes.testFlag( AudioSmem ) || m_modes.testFlag( FileOutput ) )
    {
        Q_ASSERT( m_modes.testFlag( VideoSmem ) == false );

        if ( m_outputAudioFourCC.isNull() == false )
            transcodeStr += ",acodec=" + m_outputAudioFourCC;
        if ( m_outputAudioBitrate > 0 )
            transcodeStr += ",ab=" + QString::number( m_outputAudioBitrate );
        if ( m_outputNbChannels > 0)
            transcodeStr += ",channels=" + QString::number( m_outputNbChannels );
        if ( m_outputSampleRate > 0 )
            transcodeStr += ",samplerate=" + QString::number( m_outputSampleRate );
    }
    transcodeStr += '}';
    QString     fileOutput = setupFileOutput();

    transcodeStr += fileOutput + m_smemChain;

    setOption( transcodeStr );
}

QString
VLCSourceRenderer::setupFileOutput()
{
    if ( m_modes.testFlag( FileOutput ) == false )
        return QString();
    Q_ASSERT( m_outputFileName.isNull() == false );
    QString soutConfig = ":standard{access=file,mux=ps,dst=\"";
    soutConfig += m_outputFileName;
    soutConfig += "\"}";
    return soutConfig;
}

void
VLCSourceRenderer::setOption( const QString &option )
{
    Q_ASSERT( m_media.isValid() == true );
    vlmcDebug() << m_name << "Setting option:" << option;
    m_media.addOption( option.toStdString() );
}

void
VLCSourceRenderer::start()
{
    if ( m_mediaPlayer.isPlaying() == false )
    {
        setupStreamOutput();
        m_mediaPlayer.setMedia( m_media );
        m_mediaPlayer.play();
    }
}

void
VLCSourceRenderer::stop()
{
    m_mediaPlayer.stop();
}

bool
VLCSourceRenderer::isStopped()
{
    return m_mediaPlayer.state() == libvlc_Stopped;
}

void
VLCSourceRenderer::playPause()
{
    m_mediaPlayer.pause();
}

void
VLCSourceRenderer::setPause( bool isPaused )
{
    m_mediaPlayer.setPause( isPaused );
}

void
VLCSourceRenderer::nextFrame()
{
    m_mediaPlayer.nextFrame();
}

void VLCSourceRenderer::previousFrame()
{
    vlmcWarning() << "PreviousFrame: Not implemented";
}

void
VLCSourceRenderer::setOutputWidget( void *target )
{
#if defined ( Q_OS_MAC )
    m_mediaPlayer.setNsobject( target );
#elif defined ( Q_OS_UNIX )
    m_mediaPlayer.setXwindow( reinterpret_cast<intptr_t>( target ) );
#elif defined ( Q_OS_WIN )
    m_mediaPlayer.setHwnd( target );
#endif
}

int64_t
VLCSourceRenderer::time()
{
    return m_mediaPlayer.time();
}

void
VLCSourceRenderer::setTime( int64_t time )
{
    vlmcDebug() << m_name << "Set time:" << time;
    m_mediaPlayer.setTime( time );
}

void
VLCSourceRenderer::setPosition( float position )
{
    m_mediaPlayer.setPosition( position );
}

int
VLCSourceRenderer::volume()
{
    return m_mediaPlayer.volume();
}

void VLCSourceRenderer::setVolume(int volume)
{
    m_mediaPlayer.setVolume( volume );
}

void
VLCSourceRenderer::setOutputFile(const char *path)
{
    m_modes |= FileOutput;
    m_outputFileName = path;
}

void
VLCSourceRenderer::setOutputVideoCodec( const char *fourCC )
{
    m_outputVideoFourCC = fourCC;
}

void
VLCSourceRenderer::setOutputWidth( unsigned int width )
{
    m_outputWidth = width;
}

void
VLCSourceRenderer::setOutputHeight( unsigned int height )
{
    m_outputHeight = height;
}

void
VLCSourceRenderer::setOutputFps(float fps)
{
    m_outputFps = fps;
}

void
VLCSourceRenderer::setOutputVideoBitrate(unsigned int vBitrate)
{
    m_outputVideoBitrate = vBitrate;
}

void
VLCSourceRenderer::setOutputAudioCodec(const char *fourCC)
{
    m_outputAudioFourCC = fourCC;
}

void
VLCSourceRenderer::setOutputAudioSampleRate(unsigned int sampleRate)
{
    m_outputSampleRate = sampleRate;
}

void
VLCSourceRenderer::setOutputAudioNumberChannels(unsigned int nbChannels)
{
    m_outputNbChannels = nbChannels;
}

void
VLCSourceRenderer::setOutputAudioBitrate(unsigned int aBitrate)
{
    m_outputAudioBitrate = aBitrate;
}

void
VLCSourceRenderer::enableVideoOutputToMemory( void *data, VideoOutputLockCallback lock, VideoOutputUnlockCallback unlock, bool timeSync )
{
    m_modes |= VideoSmem;
    m_smemChain = ":smem{";
    if ( timeSync == true )
        m_smemChain += "time-sync";
    else
        m_smemChain += "no-time-sync";
    m_smemChain += ",video-data=" % QString::number( reinterpret_cast<intptr_t>( data ) )
            % ",video-prerender-callback="
            % QString::number( reinterpret_cast<intptr_t>( lock ) )
            % ",video-postrender-callback="
            % QString::number( reinterpret_cast<intptr_t>( unlock ) )
            % '}';
    setOption( ":no-audio" );
    setOption( ":no-sout-audio" );
}

void
VLCSourceRenderer::enableAudioOutputToMemory(void *data, AudioOutputLockCallback lock, AudioOutputUnlockCallback unlock, bool timeSync)
{
    m_modes |= AudioSmem;
    m_smemChain = ":smem{";
    if ( timeSync == true )
        m_smemChain += "time-sync";
    else
        m_smemChain += "no-time-sync";
    m_smemChain += ",audio-data=" % QString::number( reinterpret_cast<intptr_t>( data ) )
            % ",audio-prerender-callback="
            % QString::number( reinterpret_cast<intptr_t>( lock ) )
            % ",audio-postrender-callback="
            % QString::number( reinterpret_cast<intptr_t>( unlock ) )
            % '}';
    setOption( ":no-video" );
    setOption( ":no-sout-video" );
}

void
VLCSourceRenderer::enableMemoryInput( void *data, MemoryInputLockCallback lockCallback, MemoryInputUnlockCallback unlockCallback )
{
    Q_ASSERT( m_media != nullptr );
    m_modes |= Imem;

    char        buffer[64];

    sprintf( buffer, "imem-get=%p", lockCallback );
    m_media.addOption( buffer );
    sprintf( buffer, ":imem-release=%p", unlockCallback );
    m_media.addOption( buffer );
    sprintf( buffer, ":imem-data=%p", data );
    m_media.addOption( buffer );
}

