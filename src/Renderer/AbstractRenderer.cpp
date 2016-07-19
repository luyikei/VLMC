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
#include "Backend/MLT/MLTOutput.h"

#include <QtGlobal>

AbstractRenderer::AbstractRenderer()
    : m_input( nullptr )
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

#ifdef HAVE_GUI
RendererEventWatcher*
AbstractRenderer::eventWatcher()
{
    return m_eventWatcher;
}
#endif

void
AbstractRenderer::stop()
{
    m_output->stop();
}

void
AbstractRenderer::setPosition( qint64 pos )
{
    if ( m_input )
        m_input->setPosition( pos );
}

void
AbstractRenderer::togglePlayPause()
{
    if ( m_input == nullptr || !m_output )
        return;

    if ( m_output->isStopped() )
    {
        m_output->start();
        m_input->setPause( false );
    }
    else
        m_input->playPause();
}

int
AbstractRenderer::getVolume() const
{
    return m_output->volume();
}

void
AbstractRenderer::setVolume( int volume )
{
    m_output->setVolume( volume );
}

void
AbstractRenderer::nextFrame()
{
    if ( isRendering() && m_input )
        m_input->nextFrame();
}

void
AbstractRenderer::previousFrame()
{
    if ( isRendering() && m_input )
        m_input->previousFrame();
}

qint64
AbstractRenderer::length() const
{
    if ( m_input )
        return m_input->playableLength();
    return 0;
}

qint64
AbstractRenderer::getLengthMs() const
{
    if ( m_input )
        return ( qRound64( (qreal)( m_input->playableLength() ) / m_input->fps() * 1000.0 ) );
    return 0;
}

qint64
AbstractRenderer::getCurrentFrame() const
{
    if ( m_input )
        return m_input->position();
    return 0;
}

float
AbstractRenderer::getFps() const
{
    if ( m_input )
        return m_input->fps();
    return 0.0f;
}

bool
AbstractRenderer::isPaused() const
{
    if ( m_input )
        return m_input->isPaused();
    return false;
}

bool
AbstractRenderer::isRendering() const
{
    return !m_output->isStopped();
}

void
AbstractRenderer::setInput( Backend::IInput* input )
{
    m_input = input;

    if ( m_input )
    {
        m_input->setCallback( m_eventWatcher );
        emit lengthChanged( m_input->playableLength() );
    }
    else
        emit lengthChanged( 0 );

    if ( m_output.get() != nullptr )
        m_output->connect( *m_input );
}

void
AbstractRenderer::setOutput( std::unique_ptr<Backend::IOutput> consuemr )
{
    m_output = std::move( consuemr );
    m_output->setCallback( m_eventWatcher );

    if ( m_input != nullptr )
        m_output->connect( *m_input );
}

void
AbstractRenderer::previewWidgetCursorChanged( qint64 newFrame )
{
    if ( isRendering() == true )
    {
        m_input->setPosition( newFrame );
    }
}
