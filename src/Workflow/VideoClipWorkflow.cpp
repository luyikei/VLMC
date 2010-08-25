/*****************************************************************************
 * VideoClipWorkflow.cpp : Clip workflow. Will extract a single frame from a VLCMedia
 *****************************************************************************
 * Copyright (C) 2008-2010 VideoLAN
 *
 * Authors: Hugo Beauzee-Luyssen <hugo@vlmc.org>
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

#include "Clip.h"
#include "EffectInstance.h"
#include "MainWorkflow.h"
#include "Media.h"
#include "VideoClipWorkflow.h"
#include "VLCMedia.h"
#include "WaitCondition.hpp"
#include "Workflow/Types.h"

#include <QReadWriteLock>
#include <QtDebug>

VideoClipWorkflow::VideoClipWorkflow( ClipHelper *ch ) :
        ClipWorkflow( ch ),
        m_width( 0 ),
        m_height( 0 ),
        m_renderedFrame( 0 ),
        m_lastReturnedBuffer( NULL )
{
    m_effectsLock = new QReadWriteLock();
    m_renderedFrameMutex = new QMutex();
}

VideoClipWorkflow::~VideoClipWorkflow()
{
    stop();
    qDeleteAll( m_filters );
    delete m_renderedFrameMutex;
    delete m_effectsLock;
}

void
VideoClipWorkflow::releasePrealocated()
{
    while ( m_availableBuffers.isEmpty() == false )
        delete m_availableBuffers.dequeue();
    while ( m_computedBuffers.isEmpty() == false )
        delete m_computedBuffers.dequeue();
}

void
VideoClipWorkflow::preallocate()
{
    quint32     newWidth = MainWorkflow::getInstance()->getWidth();
    quint32     newHeight = MainWorkflow::getInstance()->getHeight();
    if ( newWidth != m_width || newHeight != m_height )
    {
        m_width = newWidth;
        m_height = newHeight;
        while ( m_availableBuffers.isEmpty() == false )
            delete m_availableBuffers.dequeue();
        for ( unsigned int i = 0; i < VideoClipWorkflow::nbBuffers; ++i )
        {
            m_availableBuffers.enqueue( new Workflow::Frame( newWidth, newHeight ) );
        }
    }
}

void
VideoClipWorkflow::initVlcOutput()
{
    char        buffer[32];

    preallocate();
    m_vlcMedia->addOption( ":no-audio" );
    m_vlcMedia->addOption( ":no-sout-audio" );
    m_vlcMedia->addOption( ":sout=#transcode{}:smem" );
    m_vlcMedia->setVideoDataCtx( this );
    m_vlcMedia->setVideoLockCallback( reinterpret_cast<void*>( getLockCallback() ) );
    m_vlcMedia->setVideoUnlockCallback( reinterpret_cast<void*>( getUnlockCallback() ) );
    m_vlcMedia->addOption( ":sout-transcode-vcodec=RV32" );
    if ( m_fullSpeedRender == false )
        m_vlcMedia->addOption( ":sout-smem-time-sync" );
    else
        m_vlcMedia->addOption( ":no-sout-smem-time-sync" );

    sprintf( buffer, ":sout-transcode-width=%i", m_width );
    m_vlcMedia->addOption( buffer );
    sprintf( buffer, ":sout-transcode-height=%i", m_height );
    m_vlcMedia->addOption( buffer );
    sprintf( buffer, ":sout-transcode-fps=%f", clip()->getMedia()->fps() );
    m_vlcMedia->addOption( buffer );

    QReadLocker     lock( m_effectsLock );
    EffectsEngine::initFilters( m_filters, m_width, m_height );
}

void*
VideoClipWorkflow::getLockCallback() const
{
    return reinterpret_cast<void*>(&VideoClipWorkflow::lock);
}

void*
VideoClipWorkflow::getUnlockCallback() const
{
    return reinterpret_cast<void*>( &VideoClipWorkflow::unlock );
}

Workflow::OutputBuffer*
VideoClipWorkflow::getOutput( ClipWorkflow::GetMode mode )
{
    QMutexLocker    lock( m_renderLock );

    if ( m_lastReturnedBuffer != NULL )
    {
        m_availableBuffers.enqueue( m_lastReturnedBuffer );
        m_lastReturnedBuffer = NULL;
    }
    if ( shouldRender() == false )
        return NULL;
    if ( getNbComputedBuffers() == 0 )
        m_renderWaitCond->wait( m_renderLock );
    //Recheck again, as the WaitCondition may have been awaken when stopping.
    if ( getNbComputedBuffers() == 0 )
        return NULL;
    Workflow::Frame         *buff;
    if ( mode == ClipWorkflow::Pop )
    {
        buff = m_computedBuffers.dequeue();
        m_lastReturnedBuffer = buff;
    }
    else if ( mode == ClipWorkflow::Get )
        buff = m_computedBuffers.head();
    postGetOutput();
    return buff;
}

void
VideoClipWorkflow::lock( VideoClipWorkflow *cw, void **pp_ret, int size )
{
    Q_UNUSED( size );
    Workflow::Frame*    frame = NULL;

    cw->m_renderLock->lock();
    if ( cw->m_availableBuffers.isEmpty() == true )
        frame = new Workflow::Frame( cw->m_width, cw->m_height );
    else
        frame = cw->m_availableBuffers.dequeue();
    cw->m_computedBuffers.enqueue( frame );
    *pp_ret = frame->buffer();
}

void
VideoClipWorkflow::unlock( VideoClipWorkflow *cw, void *buffer, int width,
                           int height, int bpp, int size, qint64 pts )
{
    Q_UNUSED( buffer );
    Q_UNUSED( width );
    Q_UNUSED( height );
    Q_UNUSED( bpp );
    Q_UNUSED( size );

    cw->computePtsDiff( pts );
    Workflow::Frame     *frame = cw->m_computedBuffers.last();
    {
        QWriteLocker    lock( cw->m_effectsLock );
        quint32     *newFrame = EffectsEngine::applyFilters( cw->m_filters, frame, cw->m_renderedFrame,
                                     cw->m_renderedFrame * 1000.0 / cw->clip()->getMedia()->fps() );
        if ( newFrame != NULL )
            frame->setBuffer( newFrame );
    }
    {
        QMutexLocker    lock( cw->m_renderedFrameMutex );
        cw->m_renderedFrame++;
    }
    frame->ptsDiff = cw->m_currentPts - cw->m_previousPts;
    cw->commonUnlock();
    cw->m_renderWaitCond->wakeAll();
    cw->m_renderLock->unlock();
}

quint32
VideoClipWorkflow::getNbComputedBuffers() const
{
    return m_computedBuffers.count();
}

quint32
VideoClipWorkflow::getMaxComputedBuffers() const
{
    return VideoClipWorkflow::nbBuffers;
}

void
VideoClipWorkflow::flushComputedBuffers()
{
    QMutexLocker    lock( m_renderLock );

    while ( m_computedBuffers.isEmpty() == false )
        m_availableBuffers.enqueue( m_computedBuffers.dequeue() );
}

EffectsEngine::EffectHelper*
VideoClipWorkflow::appendEffect( Effect *effect, qint64 start, qint64 end )
{
    if ( effect->type() != Effect::Filter )
    {
        qWarning() << "VideoClipWorkflow does not handle non filter effects.";
        return NULL;
    }
    EffectInstance      *effectInstance = effect->createInstance();
    QWriteLocker    lock( m_effectsLock );
    EffectsEngine::EffectHelper *ret = new EffectsEngine::EffectHelper( effectInstance, start, end );
    m_filters.push_back( ret );
    return ret;
}

void
VideoClipWorkflow::saveEffects( QXmlStreamWriter &project ) const
{
    QReadLocker lock( m_effectsLock );
    EffectsEngine::saveFilters( m_filters, project );
}

void
VideoClipWorkflow::setTime( qint64 time, qint64 frame )
{
    {
        QMutexLocker    lock( m_renderedFrameMutex );
        m_renderedFrame = frame;
    }
    ClipWorkflow::setTime( time, frame );
}
