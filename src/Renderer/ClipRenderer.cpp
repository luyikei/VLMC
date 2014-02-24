/*****************************************************************************
 * ClipRenderer.cpp: Render from a Clip (mainly for previewing purpose)
 *****************************************************************************
 * Copyright (C) 2008-2010 VideoLAN
 *
 * Authors: Geoffroy Lacarrière <geoffroylaca@gmail.com>
 *          Hugo Beauzée-Luyssen <hugo@beauzee.fr>
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

#include <QtGlobal>
#include <QtCore/qmath.h>

#include "Clip.h"
#include "ClipRenderer.h"
#include "ISource.h"
#include "ISourceRenderer.h"
#include "Library.h"
#include "Media.h"
#include "MainWorkflow.h"
#include "RenderWidget.h"
#include "VLCMediaPlayer.h"
#include "VLCMedia.h"

ClipRenderer::ClipRenderer() :
    GenericRenderer(),
    m_clipLoaded( false ),
    m_selectedClip( NULL ),
    m_begin( 0 ),
    m_end( -1 ),
    m_mediaChanged( false )
{
}

ClipRenderer::~ClipRenderer()
{
    stop();
}

void
ClipRenderer::setClip( Clip* clip )
{
    // if the clip is different (or NULL) we have to stop playback.
    if ( m_selectedClip != NULL &&
         ( ( clip != NULL && clip->uuid() != m_selectedClip->uuid() ) || clip == NULL ) )
    {
        clipUnloaded( m_selectedClip->uuid() );
    }
    if ( clip == NULL )
    {
        m_selectedClip = NULL;
        m_clipLoaded = false;
        return ;
    }
    m_selectedClip = clip;
    if ( clip->nbFrames() == 0 )
        return ;
    updateInfos( clip );
}

void
ClipRenderer::updateInfos( Clip* clip )
{
    m_begin = clip->begin();
    m_end = clip->end();
    if ( m_isRendering == true )
        m_mediaChanged = true;
    else
        m_clipLoaded = false;
}

void
ClipRenderer::startPreview()
{
    if ( m_selectedClip == NULL || m_selectedClip->nbFrames() == 0 )
        return ;
    updateInfos( m_selectedClip );

    delete m_sourceRenderer;
    m_sourceRenderer = m_selectedClip->getMedia()->source()->createRenderer( m_eventWatcher );
    m_sourceRenderer->setOutputWidget( (void *) static_cast< RenderWidget* >( m_renderWidget )->id() );

    connect( m_eventWatcher, SIGNAL( stopped() ), this, SLOT( __videoStopped() ) );
    connect( m_eventWatcher, SIGNAL( paused() ),  this, SIGNAL( paused() ) );
    connect( m_eventWatcher, SIGNAL( playing() ), this, SIGNAL( playing() ) );
    connect( m_eventWatcher, SIGNAL( volumeChanged() ), this, SIGNAL( volumeChanged() ) );
    connect( m_eventWatcher, SIGNAL( timeChanged( qint64 ) ), this, SLOT( __timeChanged( qint64 ) ) );

    m_sourceRenderer->start();
    m_sourceRenderer->setPosition( (float)m_begin / (float)m_selectedClip->getMedia()->source()->nbFrames() );
    m_clipLoaded = true;
    m_isRendering = true;
    m_paused = false;
    m_mediaChanged = false;
}

void
ClipRenderer::stop()
{
    if ( m_clipLoaded == true && m_isRendering == true )
    {
        m_isRendering = false;
        m_sourceRenderer->stop();
        m_paused = false;
        if ( m_mediaChanged == true )
            m_clipLoaded = false;
    }
}

void
ClipRenderer::togglePlayPause( bool forcePause )
{
    if ( m_clipLoaded == false )
    {
        emit frameChanged( 0, Vlmc::Renderer );
        startPreview();
        return ;
    }
    if ( m_paused == false && m_isRendering == true )
    {
        m_sourceRenderer->playPause();
        m_paused = true;
    }
    else if ( forcePause == false )
    {
        if ( m_isRendering == false )
        {
            m_sourceRenderer->start();
            m_sourceRenderer->setPosition( m_begin / ( m_end - m_begin ) );
            m_isRendering = true;
        }
        else
            m_sourceRenderer->playPause();
        m_paused = false;
    }
}

int
ClipRenderer::getVolume() const
{
    return m_sourceRenderer->volume();
}

void ClipRenderer::setVolume( int volume )
{
    //Returns 0 if the volume was set, -1 if it was out of range
    return m_sourceRenderer->setVolume( volume );
}

void
ClipRenderer::nextFrame()
{
    if ( m_isRendering == true )
    {
        m_sourceRenderer->nextFrame();
    }
}

void
ClipRenderer::previousFrame()
{
    if ( m_isRendering == true )
    {
        if ( m_paused == false )
            togglePlayPause( true );
        /* FIXME: Implement a better way to render previous frame */
        qint64   interval =  static_cast<qint64>( qCeil(1000.0f * 2.0f / m_selectedClip->getMedia()->source()->fps() ) );
        m_sourceRenderer->setTime( m_sourceRenderer->time() - interval );
        m_sourceRenderer->nextFrame();
    }
}

qint64
ClipRenderer::length() const
{
    return m_end - m_begin;
}

qint64
ClipRenderer::getLengthMs() const
{
    if ( m_selectedClip )
        return ( qRound64( (qreal)( m_end - m_begin ) / m_selectedClip->getMedia()->source()->fps() * 1000.0 ) );
    return 0;
}

void
ClipRenderer::clipUnloaded( const QUuid& uuid )
{
    if ( m_selectedClip != NULL && m_selectedClip->uuid() == uuid )
    {
        stop();
        m_clipLoaded = false;
        m_selectedClip = NULL;
        m_isRendering = false;
        m_paused = false;
    }
}

qint64
ClipRenderer::getCurrentFrame() const
{
    if ( m_clipLoaded == false || m_isRendering == false || m_selectedClip == NULL )
        return 0;
    return qRound64( (qreal)m_sourceRenderer->time() / 1000 *
                     (qreal)m_selectedClip->getMedia()->source()->fps() );
}

float
ClipRenderer::getFps() const
{
    if ( m_selectedClip != NULL )
        return m_selectedClip->getMedia()->source()->fps();
    return 0.0f;
}

Clip*
ClipRenderer::getClip()
{
    return m_selectedClip;
}

void
ClipRenderer::previewWidgetCursorChanged( qint64 newFrame )
{
    if ( m_isRendering == true )
    {
        newFrame += m_begin;
        qint64 nbSeconds = qRound64( (qreal)newFrame / m_selectedClip->getMedia()->source()->fps() );
        m_sourceRenderer->setTime( nbSeconds * 1000 );
    }
}

/////////////////////////////////////////////////////////////////////
/////SLOTS :
/////////////////////////////////////////////////////////////////////

void
ClipRenderer::__videoStopped()
{
    m_isRendering = false;
    if ( m_mediaChanged == true )
        m_clipLoaded = false;
    emit frameChanged( 0, Vlmc::Renderer );
}

void
ClipRenderer::__timeChanged( qint64 time )
{
    float fps = m_selectedClip->getMedia()->source()->fps();
    qint64 f = qRound64( (qreal)time / 1000.0 * fps );
    if ( f >= m_end )
        return ;
    f = f - m_begin;
    emit frameChanged( f, Vlmc::Renderer );
}

