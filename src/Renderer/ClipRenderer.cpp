/*****************************************************************************
 * ClipRenderer.cpp: Render from a Clip (mainly for previewing purpose)
 *****************************************************************************
 * Copyright (C) 2008-2016 VideoLAN
 *
 * Authors: Yikei Lu    <luyikei.qmltu@gmail.com>
 *          Geoffroy Lacarrière <geoffroylaca@gmail.com>
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <QtGlobal>
#include <QtCore/qmath.h>

#include "Media/Clip.h"
#include "ClipRenderer.h"
#include "Backend/IBackend.h"
#include "Backend/MLT/MLTOutput.h"
#include "Backend/MLT/MLTInput.h"
#include "Tools/RendererEventWatcher.h"
#include "Library/Library.h"
#include "Media/Media.h"
#include "Workflow/MainWorkflow.h"
#include "Gui/preview/RenderWidget.h"

ClipRenderer::ClipRenderer() :
    AbstractRenderer(),
    m_clipLoaded( false ),
    m_selectedClip( nullptr ),
    m_mediaChanged( false )
{
}

ClipRenderer::~ClipRenderer()
{
    stop();
}

void
ClipRenderer::setClip( QSharedPointer<Clip> clip )
{
    // if the clip is different (or nullptr) we have to stop playback.
    if ( m_selectedClip != nullptr &&
         ( ( clip != nullptr && clip->uuid() != m_selectedClip->uuid() ) || clip == nullptr ) )
    {
        clipUnloaded( m_selectedClip->uuid() );
    }
    if ( clip == nullptr )
    {
        m_selectedClip.clear();
        m_clipLoaded = false;
        m_input = nullptr;
        return ;
    }
    m_selectedClip = clip;
    setInput( clip->input() );
    if ( clip->length() == 0 )
        return ;
    if ( m_output->isStopped() == true )
        m_clipLoaded = false;
    else
        m_mediaChanged = true;
}

void
ClipRenderer::startPreview()
{
    if ( m_selectedClip == nullptr || m_selectedClip->length() == 0 )
        return ;

    m_input->setPosition( 0 );
    m_input->setPause( false );
    m_output->start();

    m_clipLoaded = true;
    m_mediaChanged = false;
}

void
ClipRenderer::stop()
{
    if ( m_clipLoaded == true && isRendering() == true )
    {
        m_output->stop();
        if ( m_mediaChanged == true )
            m_clipLoaded = false;
    }
    if ( m_input )
        m_input->setPosition( 0 );
}

void
ClipRenderer::togglePlayPause()
{
    if ( m_clipLoaded == false )
    {
        emit frameChanged( 0, Vlmc::Renderer );
        startPreview();
        return ;
    }
    if ( m_output->isStopped() )
    {
        m_output->start();
        m_input->setPause( false );
    }
    else
        m_input->playPause();
}

qint64
ClipRenderer::length() const
{
    if ( m_selectedClip )
        return m_selectedClip->length();
    return 0;
}

qint64
ClipRenderer::getLengthMs() const
{
    if ( m_input )
        return m_input->length() / m_input->fps();
    return 0;
}

void
ClipRenderer::clipUnloaded( const QUuid& uuid )
{
    if ( m_selectedClip != nullptr && m_selectedClip->uuid() == uuid )
    {
        stop();
        m_clipLoaded = false;
        m_selectedClip.clear();
    }
}

QSharedPointer<Clip>
ClipRenderer::getClip()
{
    return m_selectedClip;
}

/////////////////////////////////////////////////////////////////////
/////SLOTS :
/////////////////////////////////////////////////////////////////////

void
ClipRenderer::videoStopped()
{
    if ( m_mediaChanged == true )
        m_clipLoaded = false;
}

void
ClipRenderer::positionChanged( qint64 time )
{
    emit frameChanged( time, Vlmc::Renderer );
}

