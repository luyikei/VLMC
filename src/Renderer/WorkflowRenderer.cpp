/*****************************************************************************
 * WorkflowRenderer.cpp: Allow a current workflow preview
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

//Allow PRId64 to be defined:
#define __STDC_FORMAT_MACROS

#include <QThread>
#include <QWaitCondition>
#include <inttypes.h>

#include "WorkflowRenderer.h"

#include "Project/Project.h"
#include "Media/Clip.h"
#include "EffectsEngine/EffectInstance.h"
#include "AbstractRenderer.h"
#include "Backend/IBackend.h"
#include "Backend/ISource.h"
#include "Backend/Target/FileTarget.h"
#include "Workflow/MainWorkflow.h"
#include "Settings/Settings.h"
#include "Tools/VlmcDebug.h"
#include "Workflow/Types.h"

WorkflowRenderer::WorkflowRenderer( Backend::IBackend* backend, MainWorkflow* mainWorkflow )
    : m_mainWorkflow( mainWorkflow )
    , m_stopping( false )
    , m_outputFps( 0.0f )
    , m_aspectRatio( "" )
    , m_silencedAudioBuffer( nullptr )
    , m_nbChannels( 2 )
    , m_rate( 48000 )
    , m_oldLength( 0 )
{
    m_source = backend->createMemorySource();
    m_esHandler = new EsHandler;
    m_esHandler->self = this;

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
WorkflowRenderer::setupRenderer()
{
    m_source->setWidth( m_width );
    m_source->setHeight( m_height );
    m_source->setFps( m_outputFps );
    m_source->setAspectRatio( qPrintable( m_aspectRatio ) );
    m_source->setNumberChannels( m_nbChannels );
    m_source->setSampleRate( m_rate );
    m_esHandler->fps = m_outputFps;


    delete m_sourceRenderer;
    m_sourceRenderer = m_source->createRenderer( m_eventWatcher );
    m_renderTarget->configure( m_sourceRenderer );
    m_sourceRenderer->setName( "WorkflowRenderer" );
    m_sourceRenderer->enableMemoryInput( m_esHandler, getLockCallback(), getUnlockCallback() );
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
    if ( cookie == nullptr || ( cookie[0] != WorkflowRenderer::VideoCookie &&
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

    ret = m_mainWorkflow->getOutput( Workflow::VideoTrack, m_paused );
    ptsDiff = ret->ptsDiff;
    if ( ptsDiff == 0 )
    {
        //If no ptsDiff has been computed, we have to fake it, so we compute
        //the theorical pts for one frame.
        //this is a bit hackish though... (especially regarding the "no frame computed" detection)
        ptsDiff = 1000000 / handler->fps;
    }
    m_pts = *pts = ptsDiff + m_pts;
    *buffer = ret->buffer();
    *bufferSize = ret->size();

#ifdef WITH_GUI
    auto self = handler->self;
    if ( self->m_time.isValid() == false ||
         self->m_time.elapsed() >= 1000 )
    {
        emit self->imageUpdated( (quint8*)( *buffer ) );
        self->m_time.restart();
    }
#endif

    vlmcDebug() << __func__ << "Rendered frame. pts:" << m_pts;
    return 0;
}

int
WorkflowRenderer::lockAudio( EsHandler *handler, int64_t *pts, size_t *bufferSize, const void ** buffer )
{
    qint64                              ptsDiff;
    quint32                             nbSample;
    const Workflow::Frame         *renderAudioSample;

    if ( m_stopping == false && m_paused == false )
    {
        renderAudioSample = m_mainWorkflow->getOutput( Workflow::AudioTrack, m_paused );
    }
    else
        renderAudioSample = nullptr;
    if ( renderAudioSample != nullptr )
    {
//        vlmcDebug() << "pts diff:" << renderAudioSample->ptsDiff;
        *buffer = (uchar*)renderAudioSample->buffer();
        *bufferSize = renderAudioSample->size();
        ptsDiff = renderAudioSample->ptsDiff;
    }
    else
    {
        nbSample = m_rate / handler->fps;
        unsigned int    buffSize = m_nbChannels * 2 * nbSample;
        if ( m_silencedAudioBuffer == nullptr )
            m_silencedAudioBuffer = new uint8_t[ buffSize ];
        memset( m_silencedAudioBuffer, 0, buffSize );
        *buffer = m_silencedAudioBuffer;
        *bufferSize = buffSize;
        ptsDiff = m_pts - m_audioPts;
    }
    m_audioPts = *pts = m_audioPts + ptsDiff;
    vlmcDebug() << __func__ << "Rendered audio sample. pts:" << m_audioPts;
    return 0;
}

void
WorkflowRenderer::unlock( void *, const char*, size_t, void* )
{
    // Nothing to do for now
}

void
WorkflowRenderer::start()
{
    m_isRendering = true;
    m_paused = false;
    m_stopping = false;
    m_pts = 0;
    m_audioPts = 0;
    m_mainWorkflow->startRender( m_width, m_height );
    m_sourceRenderer->start();
}

void
WorkflowRenderer::startRenderToFile( const QString& outputFileName, quint32 width, quint32 height,
                                     double fps, const QString& ar,
                                     quint32 vbitrate, quint32 abitrate )
{
    if ( m_isRendering == true )
        return ;
    m_width = width;
    m_height = height;
    m_outputFps = fps;
    m_aspectRatio = ar;

    setRenderTarget( std::unique_ptr<Backend::IRenderTarget>( new Backend::FileTarget( qPrintable( outputFileName ) ) ) );
    setupRenderer();
    m_sourceRenderer->setOutputAudioBitrate( abitrate );
    m_sourceRenderer->setOutputVideoBitrate( vbitrate );
    connect( m_mainWorkflow, &MainWorkflow::mainWorkflowEndReached, this, &WorkflowRenderer::renderComplete );
    connect( m_mainWorkflow, &MainWorkflow::mainWorkflowEndReached, this, &WorkflowRenderer::stop );
    m_mainWorkflow->setFullSpeedRender( true );
    start();
}

void
WorkflowRenderer::startPreview()
{
    if ( m_isRendering == true )
        return ;
    if ( m_mainWorkflow->getLengthFrame() <= 0 )
        return ;

    auto project = Core::instance()->project();
    m_width = project->width();
    m_height = project->height();
    m_outputFps = project->fps();
    m_aspectRatio = project->aspectRatio();

    setupRenderer();
    m_mainWorkflow->setFullSpeedRender( false );
    start();
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
WorkflowRenderer::togglePlayPause()
{
    if ( m_isRendering == false )
        startPreview();
    else
        m_paused = !m_paused;
}

void
WorkflowRenderer::stop()
{

    m_isRendering = false;
    m_paused = false;
    m_stopping = true;
    m_mainWorkflow->stopFrameComputing();
    if ( m_sourceRenderer != nullptr )
        m_sourceRenderer->stop();
    m_mainWorkflow->stop();
    delete[] m_silencedAudioBuffer;
    m_silencedAudioBuffer = nullptr;
}

int
WorkflowRenderer::getVolume() const
{
    return m_sourceRenderer->volume();
}

void
WorkflowRenderer::setVolume( int volume )
{
    //Returns 0 if the volume was set, -1 if it was out of range
    if( m_isRendering == true )
    {
        m_sourceRenderer->setVolume( volume );
    }
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
