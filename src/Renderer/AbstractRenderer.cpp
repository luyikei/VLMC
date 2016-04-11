/*****************************************************************************
 * AbstractRenderer.cpp: Describe a common behavior for every renderers
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

#include "AbstractRenderer.h"
#include <QtGlobal>

AbstractRenderer::AbstractRenderer()
    : m_sourceRenderer( nullptr )
    , m_paused( false )
    , m_isRendering( false )
{
    m_eventWatcher = new RendererEventWatcher;
}

AbstractRenderer::~AbstractRenderer()
{
    delete m_sourceRenderer;
    delete m_eventWatcher;
}

bool
AbstractRenderer::isPaused() const
{
    return m_paused;
}

bool
AbstractRenderer::isRendering() const
{
    return m_isRendering;
}

void
AbstractRenderer::setRenderTarget( std::unique_ptr<Backend::IRenderTarget> target )
{
    m_renderTarget = std::move( target );
}

#ifdef WITH_GUI
RendererEventWatcher*
AbstractRenderer::eventWatcher()
{
    return m_eventWatcher;
}
#endif
