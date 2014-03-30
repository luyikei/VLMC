/*****************************************************************************
 * ImageClipWorkflow.cpp : Will extract a frame from an image
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

#include <QMutex>
#include <QReadWriteLock>
#include <QStringBuilder>

#include "Project/Project.h"
#include "ImageClipWorkflow.h"
#include "Media/Clip.h"
#include "ClipHelper.h"
#include "Backend/ISource.h"
#include "Backend/ISourceRenderer.h"
#include "MainWorkflow.h"
#include "Media/Media.h"
#include "Settings/Settings.h"
#include "Workflow/Types.h"

ImageClipWorkflow::ImageClipWorkflow( ClipHelper *ch ) :
        ClipWorkflow( ch ),
        m_buffer( NULL )
{
    //This is used to queue the media player stopping, as it can't be asked for
    //from vlc's input thread (well it can but it will deadlock)
    connect( this, SIGNAL( computedFinished() ),
             this, SLOT( stopComputation() ), Qt::QueuedConnection );
    m_effectFrame = new Workflow::Frame;
}

ImageClipWorkflow::~ImageClipWorkflow()
{
    stop();
    delete m_effectFrame;
}

void ImageClipWorkflow::initializeInternals()
{
    m_renderer->setName( qPrintable( QString("ImageClipWorkflow " % m_clipHelper->uuid().toString() ) ) );
    m_renderer->enableVideoOutputToMemory( this, &lock, &unlock, m_fullSpeedRender );
    m_renderer->setOutputWidth( m_width );
    m_renderer->setOutputHeight( m_height );
    m_renderer->setOutputFps( (float)VLMC_PROJECT_GET_DOUBLE( "video/VLMCOutputFPS" ) );
    m_renderer->setOutputVideoCodec( "RV32" );

    m_effectFrame->resize( Project::getInstance()->workflow()->getWidth(),
                            Project::getInstance()->workflow()->getHeight() );
    m_isRendering = true;
}

void
ImageClipWorkflow::preallocate()
{

}

Workflow::OutputBuffer*
ImageClipWorkflow::getOutput( ClipWorkflow::GetMode, qint64 currentFrame )
{
    QMutexLocker    lock( m_renderLock );

    quint32 *buff = applyFilters( m_buffer, currentFrame,
                                    currentFrame * 1000.0 / clip()->getMedia()->source()->fps() );
    if ( buff != NULL )
    {
        m_effectFrame->setBuffer( buff );
        return m_effectFrame;
    }
    return m_buffer;
}

void
ImageClipWorkflow::lock(void *data, uint8_t **pp_ret, size_t )
{
    ImageClipWorkflow* cw = reinterpret_cast<ImageClipWorkflow*>( data );
    cw->m_renderLock->lock();
    if ( cw->m_buffer == NULL )
    {
        cw->m_buffer = new Workflow::Frame( Project::getInstance()->workflow()->getWidth(),
                                            Project::getInstance()->workflow()->getHeight() );
    }
    *pp_ret = (uint8_t*)cw->m_buffer->buffer();
}

void
ImageClipWorkflow::unlock( void* data, uint8_t*, int, int, int, size_t, int64_t )
{
    ImageClipWorkflow* cw = reinterpret_cast<ImageClipWorkflow*>( data );
    cw->m_renderLock->unlock();
    cw->emit computedFinished();
}

quint32
ImageClipWorkflow::getNbComputedBuffers() const
{
    QMutexLocker    lock( m_renderLock );

    if ( m_buffer != NULL )
        return 1;
    return 0;
}

quint32
ImageClipWorkflow::getMaxComputedBuffers() const
{
    return 1;
}

void
ImageClipWorkflow::stopComputation()
{
    m_renderer->stop();
}

void
ImageClipWorkflow::flushComputedBuffers()
{
}
