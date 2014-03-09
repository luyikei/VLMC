/*****************************************************************************
 * WorkflowFileRenderer.cpp: Output the workflow to a file
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

#include "vlmc.h"
#include "WorkflowFileRenderer.h"
#include "Settings/Settings.h"
#include "VLCMedia.h"
#include "VLCMediaPlayer.h"

#include <QTime>

WorkflowFileRenderer::WorkflowFileRenderer( Backend::IBackend* backend )
    : WorkflowRenderer( backend )
    ,   m_renderVideoFrame( NULL )
{
}

WorkflowFileRenderer::~WorkflowFileRenderer()
{
}

void
WorkflowFileRenderer::run( const QString& outputFileName, quint32 width,
                                       quint32 height, double fps, quint32 vbitrate,
                                       quint32 abitrate )
{
    m_mainWorkflow->setCurrentFrame( 0, Vlmc::Renderer );

    setupRenderer( width, height, fps );
    m_sourceRenderer->setOutputFile( qPrintable( outputFileName ) );
    m_sourceRenderer->setOutputAudioBitrate( abitrate );
    m_sourceRenderer->setOutputVideoBitrate( vbitrate );

    connect( m_mainWorkflow, SIGNAL( mainWorkflowEndReached() ), this, SLOT( stop() ) );
    connect( m_mainWorkflow, SIGNAL( frameChanged( qint64, Vlmc::FrameChangedReason ) ),
             this, SLOT( __frameChanged( qint64,Vlmc::FrameChangedReason ) ) );
    connect( this, SIGNAL( endReached() ), this, SIGNAL( renderComplete() ) );

    m_isRendering = true;
    m_stopping = false;
    m_paused = false;
    m_pts = 0;
    m_audioPts = 0;

    m_mainWorkflow->setFullSpeedRender( true );
    m_mainWorkflow->startRender( width, height, fps );
    //Waiting for renderers to preload some frames:
    SleepS( 1 );
    m_sourceRenderer->start();
}

float
WorkflowFileRenderer::getFps() const
{
    return m_outputFps;
}

int
WorkflowFileRenderer::lock( void *datas, const char* cookie, int64_t *dts, int64_t *pts,
                            unsigned int *flags, size_t *bufferSize, const void **buffer )
{
    int         ret = WorkflowRenderer::lock( datas, cookie, dts, pts, flags, bufferSize, buffer );
    EsHandler*  handler = reinterpret_cast<EsHandler*>( datas );
    WorkflowFileRenderer* self = static_cast<WorkflowFileRenderer*>( handler->self );

#ifdef WITH_GUI
    if ( self->m_time.isValid() == false ||
        self->m_time.elapsed() >= 1000 )
    {
        if ( self->m_renderVideoFrame == NULL )
            self->m_renderVideoFrame = new quint8[*bufferSize];
        memcpy( self->m_renderVideoFrame, (uchar*)(*buffer), *bufferSize );
        self->emit imageUpdated( self->m_renderVideoFrame );
        self->m_time.restart();
    }
#endif
    return ret;
}

void
WorkflowFileRenderer::__frameChanged( qint64 frame, Vlmc::FrameChangedReason )
{
    emit frameChanged( frame );
}

Backend::ISourceRenderer::MemoryInputLockCallback
WorkflowFileRenderer::getLockCallback()
{
    return &WorkflowFileRenderer::lock;
}

Backend::ISourceRenderer::MemoryInputUnlockCallback
WorkflowFileRenderer::getUnlockCallback()
{
    return &WorkflowRenderer::unlock;
}
