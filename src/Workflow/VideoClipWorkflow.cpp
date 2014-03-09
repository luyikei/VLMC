/*****************************************************************************
 * VideoClipWorkflow.cpp : Clip workflow. Will extract a single frame from a VLCMedia
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

#include "Media/Clip.h"
#include "EffectsEngine/EffectInstance.h"
#include "Main/Project.h"
#include "MainWorkflow.h"
#include "Media/Media.h"
#include "Backend/ISource.h"
#include "Backend/ISourceRenderer.h"
#include "Settings/Settings.h"
#include "VideoClipWorkflow.h"
#include "VLCMedia.h"
#include "Tools/VlmcDebug.h"
#include "Workflow/Types.h"

#include <QMutexLocker>
#include <QReadWriteLock>
#include <QStringBuilder>
#include <QWaitCondition>

VideoClipWorkflow::VideoClipWorkflow( ClipHelper *ch ) :
        ClipWorkflow( ch ),
        m_lastReturnedBuffer( NULL )
{
    m_effectsLock = new QReadWriteLock();
}

VideoClipWorkflow::~VideoClipWorkflow()
{
    stop();
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
    quint32     newWidth = Project::getInstance()->workflow()->getWidth();
    quint32     newHeight = Project::getInstance()->workflow()->getHeight();
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
VideoClipWorkflow::initializeInternals()
{
    initFilters();
    m_renderer->setName( qPrintable( QString("VideoClipWorkflow " % m_clipHelper->uuid().toString() ) ) );
    m_renderer->enableVideoOutputToMemory( this, &lock, &unlock, m_fullSpeedRender );
    m_renderer->setOutputWidth( m_width );
    m_renderer->setOutputHeight( m_height );
    m_renderer->setOutputFps( (float)VLMC_PROJECT_GET_DOUBLE( "video/VLMCOutputFPS" ) );
    m_renderer->setOutputVideoCodec( "RV32" );
}

Workflow::OutputBuffer*
VideoClipWorkflow::getOutput( ClipWorkflow::GetMode mode, qint64 currentFrame )
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
    {
        if ( m_renderWaitCond->wait( m_renderLock, 50 ) == false )
        {
            vlmcWarning() << "Clip workflow" << m_clipHelper->uuid() << "Timed out while waiting for a frame";
            errorEncountered();
            return NULL;
        }
        if ( shouldRender() == false )
            return NULL;
    }
    Workflow::Frame         *buff = NULL;
    if ( mode == ClipWorkflow::Pop )
    {
        buff = m_computedBuffers.dequeue();
        m_lastReturnedBuffer = buff;
    }
    else
        buff = m_computedBuffers.head();

    quint32     *newFrame = applyFilters( buff, currentFrame,
                                 currentFrame * 1000.0 / clip()->getMedia()->source()->fps() );
    if ( newFrame != NULL )
        buff->setBuffer( newFrame );

    postGetOutput();
    return buff;
}

void
VideoClipWorkflow::lock( void *data, uint8_t** p_buffer, size_t size )
{
    VideoClipWorkflow* cw = reinterpret_cast<VideoClipWorkflow*>( data );

    //Mind the fact that frame size in bytes might not be width * height * bpp
    Workflow::Frame*    frame = NULL;

    cw->m_renderLock->lock();
    if ( cw->m_availableBuffers.isEmpty() == true )
    {
        if ( Workflow::Frame::Size( cw->m_width, cw->m_height ) == size )
            frame = new Workflow::Frame( cw->m_width, cw->m_height );
        else
            frame = new Workflow::Frame( cw->m_width, cw->m_height, size );
    }
    else
    {
        frame = cw->m_availableBuffers.dequeue();
        if ( frame->size() != size )
            frame->resize( size );
    }
    cw->m_computedBuffers.enqueue( frame );
    *p_buffer = (uint8_t*)frame->buffer();
}

void
VideoClipWorkflow::unlock( void *data, uint8_t *buffer, int width,
                           int height, int bpp, size_t size, int64_t pts )
{
    Q_UNUSED( buffer );
    Q_UNUSED( width );
    Q_UNUSED( height );
    Q_UNUSED( bpp );
    Q_UNUSED( size );

    VideoClipWorkflow* cw = reinterpret_cast<VideoClipWorkflow*>( data );

    cw->computePtsDiff( pts );
    Workflow::Frame     *frame = cw->m_computedBuffers.last();
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
