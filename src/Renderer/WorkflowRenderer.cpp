/*****************************************************************************
 * WorkflowRenderer.cpp: Allow a current workflow preview
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

//Allow PRId64 to be defined:
#define __STDC_FORMAT_MACROS

#include "WorkflowRenderer.h"

#include "Clip.h"
#include "EffectInstance.h"
#include "GenericRenderer.h"
#include "IBackend.h"
#include "ISource.h"
#include "MainWorkflow.h"
#include "RenderWidget.h"
#include "SettingsManager.h"
#include "VLCMedia.h"
#include "VLCMediaPlayer.h"
#include "VlmcDebug.h"
#include "Workflow/Types.h"
#include "timeline/Timeline.h"

#include <QDomElement>
#include <QThread>
#include <QWaitCondition>
#include <inttypes.h>

WorkflowRenderer::WorkflowRenderer( Backend::IBackend* backend ) :
            m_mainWorkflow( MainWorkflow::getInstance() ),
            m_stopping( false ),
            m_outputFps( 0.0f ),
            m_aspectRatio( "" ),
            m_silencedAudioBuffer( NULL ),
            m_esHandler( NULL ),
            m_oldLength( 0 ),
            m_effectFrame( NULL )
{
    m_source = backend->createMemorySource();
}

void
WorkflowRenderer::initializeRenderer()
{
    m_esHandler = new EsHandler;
    m_esHandler->self = this;

    m_nbChannels = 2;
    m_rate = 48000;

    connect( m_mainWorkflow, SIGNAL( frameChanged( qint64, Vlmc::FrameChangedReason ) ),
             this, SIGNAL( frameChanged( qint64, Vlmc::FrameChangedReason ) ) );
    connect( m_mainWorkflow, SIGNAL( lengthChanged( qint64 ) ),
             this, SLOT(mainWorkflowLenghtChanged(qint64) ) );
}

WorkflowRenderer::~WorkflowRenderer()
{
    stop();

    delete m_esHandler;
    delete m_silencedAudioBuffer;
    delete m_source;
}

void
WorkflowRenderer::setupRenderer( quint32 width, quint32 height, double fps )
{
    m_source->setWidth( width );
    m_source->setHeight( height );
    m_source->setFps( fps );
    m_source->setAspectRatio( qPrintable( aspectRatio() ) );
    m_source->setNumberChannels( m_nbChannels );
    m_source->setSampleRate( m_rate );


    delete m_sourceRenderer;
    m_sourceRenderer = m_source->createRenderer( m_eventWatcher );
    m_sourceRenderer->enableMemoryInput( m_esHandler, getLockCallback(), getUnlockCallback() );
    m_sourceRenderer->setOutputWidget( (void *) static_cast< RenderWidget* >( m_renderWidget )->id() );
}

int
WorkflowRenderer::lock( void *data, const char* cookie, int64_t *dts, int64_t *pts,
                        unsigned int *flags, size_t *bufferSize, const void **buffer )
{
    int             ret = 1;
    EsHandler*      handler = reinterpret_cast<EsHandler*>( data );
    bool            paused = handler->self->m_paused;

    *dts = -1;
    *flags = 0;
    if ( cookie == NULL || ( cookie[0] != WorkflowRenderer::VideoCookie &&
                             cookie[0] != WorkflowRenderer::AudioCookie ) )
    {
        vlmcCritical() << "Invalid imem input cookie";
        return ret;
    }
    if ( cookie[0] == WorkflowRenderer::VideoCookie )
    {
        ret = handler->self->lockVideo( handler, pts, bufferSize, buffer );
        if ( paused == false )
            handler->self->m_mainWorkflow->nextFrame( Workflow::VideoTrack );
    }
    else if ( cookie[0] == WorkflowRenderer::AudioCookie )
    {
        ret = handler->self->lockAudio( handler, pts, bufferSize, buffer );
        if ( paused == false )
            handler->self->m_mainWorkflow->nextFrame( Workflow::AudioTrack );
    }
    else
        vlmcCritical() << "Invalid imem cookie";
    return ret;
}

int
WorkflowRenderer::lockVideo( void* data, int64_t *pts, size_t *bufferSize, const void **buffer )
{
    EsHandler*              handler = reinterpret_cast<EsHandler*>( data );
    qint64                  ptsDiff = 0;
    const Workflow::Frame   *ret;

    if ( m_stopping == true )
        return 1;

    ret = static_cast<const Workflow::Frame*>( m_mainWorkflow->getOutput( Workflow::VideoTrack, m_paused ) );
    ptsDiff = ret->ptsDiff;
    if ( ptsDiff == 0 )
    {
        //If no ptsDiff has been computed, we have to fake it, so we compute
        //the theorical pts for one frame.
        //this is a bit hackish though... (especially regarding the "no frame computed" detection)
        ptsDiff = 1000000 / handler->fps;
    }
    m_effectFrame = applyFilters( ret, m_mainWorkflow->getCurrentFrame(),
                                      m_mainWorkflow->getCurrentFrame() * 1000.0 / handler->fps );
    m_pts = *pts = ptsDiff + m_pts;
    if ( m_effectFrame != NULL )
        *buffer = m_effectFrame;
    else
        *buffer = ret->buffer();
    *bufferSize = ret->size();
    return 0;
}

int
WorkflowRenderer::lockAudio( EsHandler *handler, int64_t *pts, size_t *bufferSize, const void ** buffer )
{
    qint64                              ptsDiff;
    quint32                             nbSample;
    const Workflow::AudioSample         *renderAudioSample;

    if ( m_stopping == false && m_paused == false )
    {
        renderAudioSample = static_cast<const Workflow::AudioSample*>( m_mainWorkflow->getOutput( Workflow::AudioTrack,
                                                                                           m_paused ) );
    }
    else
        renderAudioSample = NULL;
    if ( renderAudioSample != NULL )
    {
//        vlmcDebug() << "pts diff:" << renderAudioSample->ptsDiff;
        nbSample = renderAudioSample->nbSample;
        *buffer = renderAudioSample->buff;
        *bufferSize = renderAudioSample->size;
        ptsDiff = renderAudioSample->ptsDiff;
    }
    else
    {
        nbSample = m_rate / handler->fps;
        unsigned int    buffSize = m_nbChannels * 2 * nbSample;
        if ( m_silencedAudioBuffer == NULL )
            m_silencedAudioBuffer = new uint8_t[ buffSize ];
        memset( m_silencedAudioBuffer, 0, buffSize );
        *buffer = m_silencedAudioBuffer;
        *bufferSize = buffSize;
        ptsDiff = m_pts - m_audioPts;
    }
    m_audioPts = *pts = m_audioPts + ptsDiff;
    return 0;
}

void
WorkflowRenderer::unlock( void *data, const char*, size_t, void* )
{
    EsHandler*      handler = reinterpret_cast<EsHandler*>( data );
    delete[] handler->self->m_effectFrame;
    handler->self->m_effectFrame = NULL;
}

void
WorkflowRenderer::startPreview()
{
    if ( m_mainWorkflow->getLengthFrame() <= 0 )
        return ;
    if ( paramsHasChanged( m_width, m_height, m_outputFps, m_aspectRatio ) == true )
    {
        m_width = width();
        m_height = height();
        m_outputFps = outputFps();
        m_aspectRatio = aspectRatio();
    }
    initFilters();

    setupRenderer( m_width, m_height, m_outputFps );

    m_mainWorkflow->setFullSpeedRender( false );
    m_mainWorkflow->startRender( m_width, m_height, m_outputFps );
    m_isRendering = true;
    m_paused = false;
    m_stopping = false;
    m_pts = 0;
    m_audioPts = 0;
    m_sourceRenderer->start();
}

void
WorkflowRenderer::nextFrame()
{
    if ( m_paused == true )
        m_mainWorkflow->renderOneFrame();
}

void
WorkflowRenderer::previousFrame()
{
    if ( m_paused == true )
        m_mainWorkflow->previousFrame( Workflow::VideoTrack );
}

void
WorkflowRenderer::togglePlayPause( bool forcePause )
{
    if ( m_isRendering == false && forcePause == false )
        startPreview();
    else
        internalPlayPause( forcePause );
}

void
WorkflowRenderer::internalPlayPause( bool forcePause )
{
    //If force pause is true, we just ensure that this render is paused... no need to start it.
    if ( m_isRendering == true )
    {
        if ( m_paused == true && forcePause == false )
        {
            m_paused = false;
        }
        else
        {
            if ( m_paused == false )
            {
                m_paused = true;
            }
        }
    }
}

void
WorkflowRenderer::stop()
{
    m_isRendering = false;
    m_paused = false;
    m_stopping = true;
    m_mainWorkflow->stopFrameComputing();
    m_sourceRenderer->stop();
    m_mainWorkflow->stop();
    delete[] m_silencedAudioBuffer;
    m_silencedAudioBuffer = NULL;
}

int
WorkflowRenderer::getVolume() const
{
    return m_sourceRenderer->volume();
}

void WorkflowRenderer::setVolume( int volume )
{
    //Returns 0 if the volume was set, -1 if it was out of range
    m_sourceRenderer->setVolume( volume );
}

qint64
WorkflowRenderer::getCurrentFrame() const
{
    return m_mainWorkflow->getCurrentFrame();
}

qint64
WorkflowRenderer::length() const
{
    return qRound64( (qreal)getLengthMs() / 1000.0 * (qreal)getFps() );
}

qint64
WorkflowRenderer::getLengthMs() const
{
    return m_mainWorkflow->getLengthFrame() / getFps() * 1000;
}

float
WorkflowRenderer::getFps() const
{
    return m_outputFps;
}

void
WorkflowRenderer::timelineCursorChanged( qint64 newFrame )
{
    m_mainWorkflow->setCurrentFrame( newFrame, Vlmc::TimelineCursor );
}

void
WorkflowRenderer::previewWidgetCursorChanged( qint64 newFrame )
{
    m_mainWorkflow->setCurrentFrame( newFrame, Vlmc::PreviewCursor );
}

void
WorkflowRenderer::rulerCursorChanged( qint64 newFrame )
{
    m_mainWorkflow->setCurrentFrame( newFrame, Vlmc::RulerCursor );
}

Backend::ISourceRenderer::MemoryInputLockCallback WorkflowRenderer::getLockCallback()
{
    return &WorkflowRenderer::lock;
}

Backend::ISourceRenderer::MemoryInputUnlockCallback WorkflowRenderer::getUnlockCallback()
{
    return WorkflowRenderer::unlock;
}

quint32
WorkflowRenderer::width() const
{
    return VLMC_PROJECT_GET_UINT( "video/VideoProjectWidth" );
}

quint32
WorkflowRenderer::height() const
{
    return VLMC_PROJECT_GET_UINT( "video/VideoProjectHeight" );
}

float
WorkflowRenderer::outputFps() const
{
    return VLMC_PROJECT_GET_DOUBLE( "video/VLMCOutputFPS" );
}

const QString
WorkflowRenderer::aspectRatio() const
{
    return VLMC_PROJECT_GET_STRING("video/AspectRatio");
}

bool
WorkflowRenderer::paramsHasChanged( quint32 width, quint32 height, double fps, QString aspect )
{
    quint32             newWidth = this->width();
    quint32             newHeight = this->height();
    float               newOutputFps = outputFps();
    const QString       newAspectRatio = aspectRatio();

    return ( newWidth != width || newHeight != height ||
         newOutputFps != fps || newAspectRatio != aspect );
}

void
WorkflowRenderer::saveProject( QXmlStreamWriter &project ) const
{
    project.writeStartElement( "renderer" );
    saveFilters( project );
    project.writeEndElement();
}

void
WorkflowRenderer::loadProject( const QDomElement &project )
{
    QDomElement     renderer = project.firstChildElement( "renderer" );
    if ( renderer.isNull() == true )
        return ;
    loadEffects( renderer );
}

/////////////////////////////////////////////////////////////////////
/////SLOTS :
/////////////////////////////////////////////////////////////////////

void
WorkflowRenderer::mainWorkflowLenghtChanged( qint64 /*newLength*/ )
{
//    if ( newLength > 0 )
//    {
//        if ( m_oldLength == 0 )
//        {
//            if ( m_isRendering == false )
//                startPreview();
//            m_paused = false;
//            togglePlayPause( true );
//        }
//    }
//    else if ( newLength == 0 && m_isRendering == true )
//    {
//        stop();
//    }
//    m_oldLength = newLength;
}
