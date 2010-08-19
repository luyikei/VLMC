/*****************************************************************************
 * WorkflowRenderer.cpp: Allow a current workflow preview
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

//Allow PRId64 to be defined:
#define __STDC_FORMAT_MACROS

#include "WorkflowRenderer.h"

#include "Clip.h"
#include "FilterInstance.h"
#include "GenericRenderer.h"
#include "MainWorkflow.h"
#include "SettingsManager.h"
#include "VLCMedia.h"
#include "VLCMediaPlayer.h"
#include "Workflow/Types.h"
#include "timeline/Timeline.h"

#include <QDomElement>
#include <QtDebug>
#include <QThread>
#include <QWaitCondition>
#include <inttypes.h>

WorkflowRenderer::WorkflowRenderer() :
            m_mainWorkflow( MainWorkflow::getInstance() ),
            m_media( NULL ),
            m_stopping( false ),
            m_outputFps( 0.0f ),
            m_width( 0 ),
            m_height( 0 ),
            m_silencedAudioBuffer( NULL ),
            m_esHandler( NULL ),
            m_oldLength( 0 )
{
    m_effectsLock = new QReadWriteLock;
}

void    WorkflowRenderer::initializeRenderer()
{
    m_esHandler = new EsHandler;
    m_esHandler->self = this;

    m_nbChannels = 2;
    m_rate = 48000;

     //Workflow part
    connect( m_mainWorkflow, SIGNAL( mainWorkflowEndReached() ), this, SLOT( __endReached() ), Qt::QueuedConnection );
    connect( m_mainWorkflow, SIGNAL( frameChanged( qint64, MainWorkflow::FrameChangedReason ) ),
             this, SIGNAL( frameChanged( qint64, MainWorkflow::FrameChangedReason ) ) );
    connect( m_mainWorkflow, SIGNAL( lengthChanged( qint64 ) ),
             this, SLOT(mainWorkflowLenghtChanged(qint64) ) );
    //Media player part: to update PreviewWidget
    connect( m_mediaPlayer, SIGNAL( playing() ),    this,   SIGNAL( playing() ), Qt::DirectConnection );
    connect( m_mediaPlayer, SIGNAL( paused() ),     this,   SIGNAL( paused() ), Qt::DirectConnection );
    connect( m_mediaPlayer, SIGNAL( errorEncountered() ), this, SLOT( errorEncountered() ) );
    //FIXME:: check if this doesn't require Qt::QueuedConnection
    connect( m_mediaPlayer, SIGNAL( stopped() ),    this,   SIGNAL( endReached() ) );
}

WorkflowRenderer::~WorkflowRenderer()
{
    killRenderer();

    if ( m_esHandler )
        delete m_esHandler;
    if ( m_media )
        delete m_media;
    if ( m_silencedAudioBuffer )
        delete m_silencedAudioBuffer;
    delete m_effectsLock;
}

void
WorkflowRenderer::setupRenderer( quint32 width, quint32 height, double fps )
{
    char        videoString[512];
    char        inputSlave[256];
    char        audioParameters[256];
    char        buffer[64];

    m_esHandler->fps = fps;
    //Clean any previous render.

    sprintf( videoString, "width=%i:height=%i:dar=%s:fps=%s:cookie=0:codec=%s:cat=2:caching=0",
             width, height, "16/9", "30/1", "RV32" );
    sprintf( audioParameters, "cookie=1:cat=1:codec=f32l:samplerate=%u:channels=%u:caching=0",
                m_rate, m_nbChannels );
    strcpy( inputSlave, ":input-slave=imem://" );
    strcat( inputSlave, audioParameters );

    if ( m_media != NULL )
        delete m_media;
    m_media = new LibVLCpp::Media( "imem://" + QString( videoString ) );
    m_media->addOption( inputSlave );

    sprintf( buffer, "imem-get=%"PRId64, (intptr_t)getLockCallback() );
    m_media->addOption( buffer );
    sprintf( buffer, ":imem-release=%"PRId64, (intptr_t)getUnlockCallback() );
    m_media->addOption( buffer );
    sprintf( buffer, ":imem-data=%"PRId64, (intptr_t)m_esHandler );
    m_media->addOption( buffer );
    m_media->addOption( ":text-renderer dummy" );
}

int
WorkflowRenderer::lock( void *datas, const char* cookie, qint64 *dts, qint64 *pts,
                        quint32 *flags, size_t *bufferSize, const void **buffer )
{
    int             ret = 1;
    EsHandler*      handler = reinterpret_cast<EsHandler*>( datas );
    bool            paused = handler->self->m_paused;

    *dts = -1;
    *flags = 0;
    if ( cookie == NULL || ( cookie[0] != WorkflowRenderer::VideoCookie &&
                             cookie[0] != WorkflowRenderer::AudioCookie ) )
    {
        qCritical() << "Invalid imem input cookie";
        return ret;
    }
    if ( cookie[0] == WorkflowRenderer::VideoCookie )
    {
        ret = handler->self->lockVideo( handler, pts, bufferSize, buffer );
        if ( paused == false )
            handler->self->m_mainWorkflow->nextFrame( MainWorkflow::VideoTrack );
    }
    else if ( cookie[0] == WorkflowRenderer::AudioCookie )
    {
        ret = handler->self->lockAudio( handler, pts, bufferSize, buffer );
        if ( paused == false )
            handler->self->m_mainWorkflow->nextFrame( MainWorkflow::AudioTrack );
    }
    else
        qCritical() << "Invalid imem cookie";
    return ret;
}

int
WorkflowRenderer::lockVideo( EsHandler *handler, qint64 *pts, size_t *bufferSize, const void **buffer )
{
    qint64                          ptsDiff = 0;
    MainWorkflow::OutputBuffers*    ret;

    if ( m_stopping == true )
        return 1;

    ret = m_mainWorkflow->getOutput( MainWorkflow::VideoTrack, m_paused );
    ptsDiff = ret->video->ptsDiff;
    if ( ptsDiff == 0 )
    {
        //If no ptsDiff has been computed, we have to fake it, so we compute
        //the theorical pts for one frame.
        //this is a bit hackish though... (especially regarding the "no frame computed" detection)
        ptsDiff = 1000000 / handler->fps;
    }
    {
        QReadLocker lock( m_effectsLock );
        EffectsEngine::applyFilters( m_filters, ret->video,
                                     m_mainWorkflow->getCurrentFrame(),
                                     m_mainWorkflow->getCurrentFrame() * 1000.0 / handler->fps );
    }
    m_pts = *pts = ptsDiff + m_pts;
    *buffer = ret->video->buffer();
    *bufferSize = ret->video->size();
    return 0;
}

int
WorkflowRenderer::lockAudio( EsHandler *handler, qint64 *pts, size_t *bufferSize, const void ** buffer )
{
    qint64                              ptsDiff;
    quint32                             nbSample;
    Workflow::AudioSample               *renderAudioSample;

    if ( m_stopping == false && m_paused == false )
    {
        MainWorkflow::OutputBuffers* ret = m_mainWorkflow->getOutput( MainWorkflow::AudioTrack,
                                                                      m_paused );
        renderAudioSample = ret->audio;
    }
    else
        renderAudioSample = NULL;
    if ( renderAudioSample != NULL )
    {
//        qDebug() << "pts diff:" << renderAudioSample->ptsDiff;
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

void    WorkflowRenderer::unlock( void*, const char*, size_t, void* )
{
}

void        WorkflowRenderer::startPreview()
{
    if ( m_mainWorkflow->getLengthFrame() <= 0 )
        return ;
    if ( paramsHasChanged( m_width, m_height, m_outputFps ) == true )
    {
        m_width = width();
        m_height = height();
        m_outputFps = outputFps();
        setupRenderer( m_width, m_height, m_outputFps );
    }
    QReadLocker     lock( m_effectsLock );
    EffectsEngine::initFilters( m_filters, m_width, m_height );

    //Deactivating vlc's keyboard inputs.
    m_mediaPlayer->setKeyInput( false );
    m_mediaPlayer->setMedia( m_media );

    m_mainWorkflow->setFullSpeedRender( false );
    m_mainWorkflow->startRender( m_width, m_height );
    m_isRendering = true;
    m_paused = false;
    m_stopping = false;
    m_pts = 0;
    m_audioPts = 0;
    m_mediaPlayer->play();
}

void        WorkflowRenderer::nextFrame()
{
    if ( m_paused == true )
        m_mainWorkflow->renderOneFrame();
}

void        WorkflowRenderer::previousFrame()
{
    if ( m_paused == true )
        m_mainWorkflow->previousFrame( MainWorkflow::VideoTrack );
}

void        WorkflowRenderer::togglePlayPause( bool forcePause )
{
    if ( m_isRendering == false && forcePause == false )
        startPreview();
    else
        internalPlayPause( forcePause );
}

void        WorkflowRenderer::internalPlayPause( bool forcePause )
{
    //If force pause is true, we just ensure that this render is paused... no need to start it.
    if ( m_isRendering == true )
    {
        if ( m_paused == true && forcePause == false )
        {
            m_paused = false;
            emit playing();
        }
        else
        {
            if ( m_paused == false )
            {
                m_paused = true;
                emit paused();
            }
        }
    }
}

void        WorkflowRenderer::stop()
{
    //Since we want permanent render (to have a permanent render update, we shouldn't
    //stop, but pause
//    togglePlayPause( true );
//    m_mainWorkflow->setCurrentFrame( 0, MainWorkflow::Renderer );
    killRenderer();
}

void
WorkflowRenderer::killRenderer()
{
    m_isRendering = false;
    m_paused = false;
    m_stopping = true;
    m_mainWorkflow->stopFrameComputing();
    m_mediaPlayer->stop();
    m_mainWorkflow->stop();
    delete[] m_silencedAudioBuffer;
    m_silencedAudioBuffer = NULL;
}

qint64      WorkflowRenderer::getCurrentFrame() const
{
    return m_mainWorkflow->getCurrentFrame();
}

qint64      WorkflowRenderer::getLength() const
{
    return qRound64( (qreal)getLengthMs() / 1000.0 * (qreal)getFps() );
}

qint64      WorkflowRenderer::getLengthMs() const
{
    return m_mainWorkflow->getLengthFrame() / getFps() * 1000;
}

float       WorkflowRenderer::getFps() const
{
    return m_outputFps;
}

void        WorkflowRenderer::timelineCursorChanged( qint64 newFrame )
{
    m_mainWorkflow->setCurrentFrame( newFrame, MainWorkflow::TimelineCursor );
}

void        WorkflowRenderer::previewWidgetCursorChanged( qint64 newFrame )
{
    m_mainWorkflow->setCurrentFrame( newFrame, MainWorkflow::PreviewCursor );
}

void        WorkflowRenderer::rulerCursorChanged( qint64 newFrame )
{
    m_mainWorkflow->setCurrentFrame( newFrame, MainWorkflow::RulerCursor );
}

void*   WorkflowRenderer::getLockCallback()
{
    return (void*)&WorkflowRenderer::lock;
}

void*   WorkflowRenderer::getUnlockCallback()
{
    return (void*)&WorkflowRenderer::unlock;
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

bool
WorkflowRenderer::paramsHasChanged( quint32 width, quint32 height, double fps )
{
    quint32             newWidth = this->width();
    quint32             newHeight = this->height();
    float               newOutputFps = outputFps();

    return ( newWidth != width || newHeight != height ||
         newOutputFps != fps );
}

void
WorkflowRenderer::appendEffect( Effect *effect, qint64 start, qint64 end )
{
    if ( effect->type() != Effect::Filter )
    {
        qWarning() << "WorkflowRenderer does not handle non filter effects.";
        return ;
    }
    FilterInstance  *filterInstance = static_cast<FilterInstance*>( effect->createInstance() );

    if ( isRendering() == true )
        filterInstance->init( m_width, m_height );
    QWriteLocker    lock( m_effectsLock );
    m_filters.push_back( new EffectsEngine::FilterHelper( filterInstance, start, end ) );
}

void
WorkflowRenderer::saveProject( QXmlStreamWriter &project ) const
{
    project.writeStartElement( "renderer" );
    {
        QReadLocker     lock( m_effectsLock );
        EffectsEngine::saveFilters( m_filters, project );
    }
    project.writeEndElement();
}

void
WorkflowRenderer::loadProject( const QDomElement &project )
{
    QDomElement     renderer = project.firstChildElement( "renderer" );
    if ( renderer.isNull() == true )
        return ;
    QDomElement     effects = renderer.firstChildElement( "effects" );
    if ( effects.isNull() == true )
        return ;
    QDomElement     effect = effects.firstChildElement( "effect" );
    while ( effect.isNull() == false )
    {
        if ( effect.hasAttribute( "name" ) == true &&
             effect.hasAttribute( "start" ) == true &&
             effect.hasAttribute( "end" ) == true )
        {
            Effect  *e = EffectsEngine::getInstance()->effect( effect.attribute( "name" ) );
            if ( e != NULL )
                appendEffect( e, effect.attribute( "start" ).toLongLong(),
                              effect.attribute( "end" ).toLongLong() );
            else
                qCritical() << "Renderer: Can't load effect" << effect.attribute( "name" );
        }
        effect = effect.nextSiblingElement();
    }
}

/////////////////////////////////////////////////////////////////////
/////SLOTS :
/////////////////////////////////////////////////////////////////////

void        WorkflowRenderer::__endReached()
{
    stop();
    emit endReached();
}

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

void
WorkflowRenderer::errorEncountered()
{
    stop();
    emit error();
}
