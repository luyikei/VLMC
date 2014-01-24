/*****************************************************************************
 * VideoClipWorkflow.cpp : Clip workflow. Will extract a single frame from a VLCMedia
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

#include "Clip.h"
#include "EffectInstance.h"
#include "MainWorkflow.h"
#include "Media.h"
#include "SettingsManager.h"
#include "VideoClipWorkflow.h"
#include "VLCMedia.h"
#include "Workflow/Types.h"

#include <QMutexLocker>
#include <QReadWriteLock>
#include <QStringBuilder>
#include <QWaitCondition>

#include <QtDebug>

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
VideoClipWorkflow::initializeVlcOutput()
{
    preallocate();
    m_vlcMedia->addOption(":no-audio");
    m_vlcMedia->addOption(":no-sout-audio");
    initFilters();
}

QString
VideoClipWorkflow::createSoutChain() const
{
    QString chain = ":sout=#transcode{vcodec=RV32,fps=";

    chain += QString::number( VLMC_PROJECT_GET_DOUBLE( "video/VLMCOutputFPS" ) )
            % ",width=" % QString::number( m_width ) % ",height="
            % QString::number( m_height )
            % "}:smem{";
    if ( m_fullSpeedRender == false )
        chain += "time-sync";
    else
        chain += "no-time-sync";
    chain += ",video-data=" % QString::number( reinterpret_cast<intptr_t>( this ) )
            % ",video-prerender-callback="
            % QString::number( reinterpret_cast<intptr_t>( getLockCallback() ) )
            % ",video-postrender-callback="
            % QString::number( reinterpret_cast<intptr_t>( getUnlockCallback() ) )
            % '}';

    return chain;
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
        m_renderWaitCond->wait( m_renderLock );
    //Recheck again, as the WaitCondition may have been awaken when stopping.
    if ( getNbComputedBuffers() == 0 )
        return NULL;
    Workflow::Frame         *buff = NULL;
    if ( mode == ClipWorkflow::Pop )
    {
        buff = m_computedBuffers.dequeue();
        m_lastReturnedBuffer = buff;
    }
    else
        buff = m_computedBuffers.head();

    quint32     *newFrame = applyFilters( buff, currentFrame,
                                 currentFrame * 1000.0 / clip()->getMedia()->fps() );
    if ( newFrame != NULL )
        buff->setBuffer( newFrame );

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
