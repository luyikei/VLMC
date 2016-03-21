/*****************************************************************************
 * GenericRenderer.cpp: Describe a common behavior for every renderers
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

#include "GenericRenderer.h"
#include "preview/RenderWidget.h"
#include <QtGlobal>

GenericRenderer::GenericRenderer()
    : m_sourceRenderer( NULL )
    , m_paused( false )
{
    m_eventWatcher = new RendererEventWatcher;
}

GenericRenderer::~GenericRenderer()
{
    delete m_sourceRenderer;
    delete m_eventWatcher;
}

#ifdef WITH_GUI
void
GenericRenderer::setRenderWidget(QWidget *renderWidget)
{
    m_renderWidget = renderWidget;
}

RendererEventWatcher*
GenericRenderer::eventWatcher()
{
    return m_eventWatcher;
}
#endif
