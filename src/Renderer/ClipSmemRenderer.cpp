/*****************************************************************************
 * ClipSmemRenderer.cpp: Render from a Clip
 *****************************************************************************
 * Copyright (C) 2008-2016 VideoLAN
 *
 * Authors: Hugo Beauzée-Luyssen <hugo@beauzee.fr>
 *          Yikei Lu <luyikei.qmltu@gmail.com>
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

#include <QMutex>
#include <QWaitCondition>

#include "ClipSmemRenderer.h"
#include "Backend/ISource.h"
#include "Backend/ISourceRenderer.h"
#include "Project/Project.h"
#include "Tools/VlmcDebug.h"
#include "Media/Clip.h"
#include "Media/Media.h"
#include "Main/Core.h"

ClipSmemRenderer::ClipSmemRenderer( Clip* clip, quint32 width, quint32 height, bool fullSpeedRender )
    : m_eventWatcher( new RendererEventWatcher )
    , m_width( width )
    , m_height( height )
{
    m_renderer = clip->media()->source()->createRenderer( m_eventWatcher );
    m_renderer->setName( qPrintable( QString( "ClipSmemRenderer " ) + clip->uuid().toString() ) );

    if ( clip->formats() & Clip::Audio )
    {
        m_renderer->setOutputAudioCodec( "f32l" );
        m_renderer->setOutputAudioNumberChannels( 2 );
        m_renderer->setOutputAudioSampleRate( 48000 );
    }
    if ( clip->formats() & Clip::Video )
    {
        m_renderer->setOutputWidth( m_width );
        m_renderer->setOutputHeight( m_height );
        m_renderer->setOutputFps( Core::instance()->project()->fps() );
        m_renderer->setOutputVideoCodec( "RV32" );
    }

    if ( clip->formats() == ( Clip::Video | Clip::Audio ) )
        m_renderer->enableOutputToMemory( this, this, &videoLock, &videoUnlock, &audioLock, &audioUnlock, fullSpeedRender );
    else if ( clip->formats() & Clip::Video )
        m_renderer->enableVideoOutputToMemory( this, &videoLock, &videoUnlock, fullSpeedRender );
    else if ( clip->formats() & Clip::Audio )
        m_renderer->enableAudioOutputToMemory( this, &audioLock, &audioUnlock, fullSpeedRender );

    for ( int i = 0; i < Workflow::NbTrackType; ++i )
    {
       m_renderLock[i] = new QMutex;
       m_renderWaitCond[i] = new QWaitCondition;
    }

    connect( this, &ClipSmemRenderer::bufferReachedMax, this, &ClipSmemRenderer::pause, Qt::QueuedConnection );

    preallocate();
}

ClipSmemRenderer::~ClipSmemRenderer()
{
    stop();
    delete m_eventWatcher;
    delete m_renderer;
    releasePrealocated();
    for ( int i = 0; i < Workflow::NbTrackType; ++i )
    {
       delete m_renderLock[i];
       delete m_renderWaitCond[i];
    }
}

RendererEventWatcher*
ClipSmemRenderer::eventWatcher()
{
    return m_eventWatcher;
}

constexpr qint32
ClipSmemRenderer::maxNumBuffers( Workflow::TrackType trackType )
{
    return ( trackType == Workflow::AudioTrack ) ? 256 : 3 * 30;
}

Workflow::Frame*
ClipSmemRenderer::getOutput( Workflow::TrackType trackType, ClipSmemRenderer::GetMode mode, qint64 currentFrame )
{
    Q_UNUSED( currentFrame )

    QMutexLocker    lock( m_renderLock[trackType] );

    if ( m_lastReturnedBuffer[trackType] != nullptr )
    {
        m_availableBuffers[trackType].enqueue( m_lastReturnedBuffer[trackType] );
        m_lastReturnedBuffer[trackType] = nullptr;
    }
    if ( m_computedBuffers[trackType].count() == 0 )
    {
        if ( m_renderWaitCond[trackType]->wait( m_renderLock[trackType], 100 ) == false )
        {
            vlmcWarning() << "ClipSmemRenderer: Timed out while waiting for a frame";
            return nullptr;
        }
    }
    Workflow::Frame         *buff = nullptr;
    if ( mode == GetMode::Pop )
    {
        buff = m_computedBuffers[trackType].dequeue();
        m_lastReturnedBuffer[trackType] = buff;
    }
    else
        buff = m_computedBuffers[trackType].head();

    //If we're running out of computed buffers, refill our stack.
    if ( m_computedBuffers[trackType].count() < maxNumBuffers( trackType ) / 3 )
        setPause( false );
    //Don't test using availableBuffer, as it may evolve if a buffer is required while
    //no one is available : we would spawn a new buffer, thus modifying the number of available buffers
    else if ( m_computedBuffers[trackType].count() >= maxNumBuffers( trackType ) )
    {
        // It's OK to check from here: if getOutput is not called, it means the clipworkflow is
        // stopped or preloading, in which case, we don't care about the buffer queue growing uncontrolled
        setPause( true );
    }

    // Fail safe
    if ( buff->buffer() == nullptr )
        return nullptr;

    return buff;
}

void
ClipSmemRenderer::preallocate()
{
    for ( int i = 0; i < Workflow::NbTrackType; ++i )
    {
        QMutexLocker        lock( m_renderLock[i] );
        while ( m_availableBuffers[i].isEmpty() == false )
            delete m_availableBuffers[i].dequeue();
        for ( int j = 0; j < maxNumBuffers( (Workflow::TrackType)i ); ++j )
            m_availableBuffers[i].enqueue( new Workflow::Frame( 0 ) ); // Don't allocate too much memory.
        m_lastReturnedBuffer[i] = nullptr;
    }
}

void
ClipSmemRenderer::releasePrealocated()
{
    for ( int i = 0; i < Workflow::NbTrackType; ++i )
    {
        QMutexLocker        lock( m_renderLock[i] );
        while ( m_availableBuffers[i].isEmpty() == false )
            delete m_availableBuffers[i].dequeue();
        while ( m_computedBuffers[i].isEmpty() == false )
            delete m_computedBuffers[i].dequeue();
    }
}

void
ClipSmemRenderer::flushComputedBuffers()
{
    for ( int i = 0; i < Workflow::NbTrackType; ++i )
    {
        QMutexLocker        lock( m_renderLock[i] );
        while ( m_computedBuffers[i].isEmpty() == false )
            m_availableBuffers[i].enqueue( m_computedBuffers[i].dequeue() );
    }
}

void
ClipSmemRenderer::stop()
{
    if ( m_renderer != nullptr )
        m_renderer->stop();
}

void
ClipSmemRenderer::start()
{
    if ( m_renderer != nullptr )
        m_renderer->start();
}

void
ClipSmemRenderer::setPause( bool val )
{
    if ( m_renderer != nullptr )
    {
        m_renderer->setPause( val );
        if ( val )
            vlmcWarning() << "ClipSmemRenderer was paused unexpectedly";
    }
}

void
ClipSmemRenderer::setTime( int64_t time )
{
    if ( m_renderer != nullptr )
        m_renderer->setTime( time );
}

void
ClipSmemRenderer::pause()
{
    if ( m_renderer != nullptr ) {
        m_renderer->setPause( true );
        vlmcWarning() << "ClipSmemRenderer was paused unexpectedly";
    }
}


void
ClipSmemRenderer::audioLock( void *data, quint8 **pcm_buffer, size_t size )
{
    ClipSmemRenderer* renderer = reinterpret_cast<ClipSmemRenderer*>( data );

    Workflow::Frame*    frame = nullptr;

    renderer->m_renderLock[Workflow::AudioTrack]->lock();
    if ( renderer->m_availableBuffers[Workflow::AudioTrack].isEmpty() == true )
        frame = new Workflow::Frame( size );
    else
    {
        frame = renderer->m_availableBuffers[Workflow::AudioTrack].dequeue();
        if ( frame->size() < size )
            frame->resize( size );
    }
    renderer->m_computedBuffers[Workflow::AudioTrack].enqueue( frame );
    *pcm_buffer = (uint8_t*)frame->buffer();
}

void
ClipSmemRenderer::audioUnlock( void *data, uint8_t *pcm_buffer, unsigned int channels,
                               unsigned int rate, unsigned int nb_samples, unsigned int bits_per_sample,
                               size_t size, int64_t pts )
{
    Q_UNUSED( pcm_buffer )
    Q_UNUSED( channels )
    Q_UNUSED( rate )
    Q_UNUSED( nb_samples )
    Q_UNUSED( bits_per_sample )
    Q_UNUSED( size )

    ClipSmemRenderer* renderer = reinterpret_cast<ClipSmemRenderer*>( data );

    Workflow::Frame     *frame = renderer->m_computedBuffers[Workflow::AudioTrack].last();
    frame->setPts( pts );
    renderer->m_renderWaitCond[Workflow::AudioTrack]->wakeAll();
    if ( maxNumBuffers( Workflow::AudioTrack ) <= renderer->m_computedBuffers[Workflow::AudioTrack].count() )
        emit renderer->bufferReachedMax();
    renderer->m_renderLock[Workflow::AudioTrack]->unlock();

}

void
ClipSmemRenderer::videoLock( void *data, uint8_t **p_buffer, size_t size )
{
    ClipSmemRenderer* renderer = reinterpret_cast<ClipSmemRenderer*>( data );

    //Mind the fact that frame size in bytes might not be width * height * bpp
    Workflow::Frame*    frame = nullptr;

    renderer->m_renderLock[Workflow::VideoTrack]->lock();
    if ( renderer->m_availableBuffers[Workflow::VideoTrack].isEmpty() == true )
    {
        frame = new Workflow::Frame( size );
    }
    else
    {
        frame = renderer->m_availableBuffers[Workflow::VideoTrack].dequeue();
        if ( frame->size() < size )
            frame->resize( size );
    }
    renderer->m_computedBuffers[Workflow::VideoTrack].enqueue( frame );
    *p_buffer = (uint8_t*)frame->buffer();
}

void
ClipSmemRenderer::videoUnlock( void *data, uint8_t *buffer, int width, int height,
                               int bpp, size_t size, int64_t pts )
{
    Q_UNUSED( buffer )
    Q_UNUSED( width )
    Q_UNUSED( height )
    Q_UNUSED( bpp )
    Q_UNUSED( size )

    ClipSmemRenderer* renderer = reinterpret_cast<ClipSmemRenderer*>( data );

    Workflow::Frame     *frame = renderer->m_computedBuffers[Workflow::VideoTrack].last();
    frame->setPts( pts );
    renderer->m_renderWaitCond[Workflow::VideoTrack]->wakeAll();
    if ( maxNumBuffers( Workflow::VideoTrack ) <= renderer->m_computedBuffers[Workflow::VideoTrack].count() )
        emit renderer->bufferReachedMax();
    renderer->m_renderLock[Workflow::VideoTrack]->unlock();

}
