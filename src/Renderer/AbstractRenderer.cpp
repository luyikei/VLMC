/*****************************************************************************
 * AbstractRenderer.cpp: Describe a common behavior for every renderers
 *****************************************************************************
 * Copyright (C) 2008-2016 VideoLAN
 *
 * Authors: Yikei Lu    <luyikei.qmltu@gmail.com>
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


#include "AbstractRenderer.h"

#include "Tools/RendererEventWatcher.h"
#include "Backend/MLT/MLTConsumer.h"

#include <QtGlobal>

AbstractRenderer::AbstractRenderer()
    : m_producer( nullptr )
{
    m_eventWatcher = new RendererEventWatcher;
    connect( m_eventWatcher, &RendererEventWatcher::stopped, this, &AbstractRenderer::stop );
    connect( m_eventWatcher, &RendererEventWatcher::positionChanged, this, [this]( qint64 pos ){ emit frameChanged( pos, Vlmc::Renderer ); } );
    connect( m_eventWatcher, &RendererEventWatcher::lengthChanged, this, &AbstractRenderer::lengthChanged );
    connect( m_eventWatcher, &RendererEventWatcher::endReached, this, &AbstractRenderer::stop );
}

AbstractRenderer::~AbstractRenderer()
{
    stop();
    delete m_eventWatcher;
}

#ifdef WITH_GUI
RendererEventWatcher*
AbstractRenderer::eventWatcher()
{
    return m_eventWatcher;
}
#endif

void
AbstractRenderer::stop()
{
    m_consumer->stop();
}

void
AbstractRenderer::setPosition( qint64 pos )
{
    if ( m_producer )
        m_producer->setPosition( pos );
}

void
AbstractRenderer::togglePlayPause()
{
    if ( m_producer == nullptr || m_consumer.get() == nullptr )
        return;

    if ( m_consumer->isStopped() )
    {
        m_consumer->start();
        m_producer->setPause( false );
    }
    else
        m_producer->playPause();
}

int
AbstractRenderer::getVolume() const
{
    return m_consumer->volume();
}

void
AbstractRenderer::setVolume( int volume )
{
    m_consumer->setVolume( volume );
}

void
AbstractRenderer::nextFrame()
{
    if ( isRendering() && m_producer )
        m_producer->nextFrame();
}

void
AbstractRenderer::previousFrame()
{
    if ( isRendering() && m_producer )
        m_producer->previousFrame();
}

qint64
AbstractRenderer::length() const
{
    if ( m_producer )
        return m_producer->playableLength();
    return 0;
}

qint64
AbstractRenderer::getLengthMs() const
{
    if ( m_producer )
        return ( qRound64( (qreal)( m_producer->playableLength() ) / m_producer->fps() * 1000.0 ) );
    return 0;
}

qint64
AbstractRenderer::getCurrentFrame() const
{
    if ( m_producer )
        return m_producer->position();
    return 0;
}

float
AbstractRenderer::getFps() const
{
    if ( m_producer )
        return m_producer->fps();
    return 0.0f;
}

bool
AbstractRenderer::isPaused() const
{
    if ( m_producer )
        return m_producer->isPaused();
    return false;
}

bool
AbstractRenderer::isRendering() const
{
    return !m_consumer->isStopped();
}

void
AbstractRenderer::setProducer( Backend::IProducer* producer )
{
    m_producer = producer;

    if ( m_producer )
    {
        m_producer->setCallback( m_eventWatcher );
        emit lengthChanged( m_producer->playableLength() );
    }
    else
        emit lengthChanged( 0 );

    if ( m_consumer.get() != nullptr )
        m_consumer->connect( *m_producer );
}

void
AbstractRenderer::setConsumer( std::unique_ptr<Backend::IConsumer> consuemr )
{
    m_consumer = std::move( consuemr );
    m_consumer->setCallback( m_eventWatcher );

    if ( m_producer != nullptr )
        m_consumer->connect( *m_producer );
}

void
AbstractRenderer::previewWidgetCursorChanged( qint64 newFrame )
{
    if ( isRendering() == true )
    {
        m_producer->setPosition( newFrame );
    }
}
