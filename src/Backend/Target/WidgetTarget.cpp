/*****************************************************************************
 * WidgetTarget.cpp: RenderTarget for Widget
 *****************************************************************************
 * Copyright (C) 2008-2016 VideoLAN
 *
 * Authors: Yikei Lu <luyikei.qmltu@gmail.com>
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

#include "WidgetTarget.h"

#include "Backend/ISourceRenderer.h"

using namespace Backend;

WidgetTarget::WidgetTarget( void* id )
    : m_id( reinterpret_cast<quintptr>( id ) )
{
}

void
WidgetTarget::configure( ISourceRenderer *renderer )
{
    renderer->setOutputWidget( reinterpret_cast<void*>( m_id ) );
}
