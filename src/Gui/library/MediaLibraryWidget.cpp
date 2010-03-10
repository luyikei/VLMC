/*****************************************************************************
 * MediaLibraryWidget.cpp
 *****************************************************************************
 * Copyright (C) 2008-2010 VideoLAN
 *
 * Authors: Thomas Boquet <thomas.boquet@gmail.com>
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

#include "MediaLibraryWidget.h"
#include "StackViewController.h"
#include "MediaListView.h"

#include "Media.h"
#include "Clip.h"

MediaLibraryWidget::MediaLibraryWidget( QWidget* parent ) : QWidget( parent )
{
    Library*  library = Library::getInstance();

    m_nav = new StackViewController( this );
    MediaListView* list = new MediaListView( m_nav, library );
    //Media
    connect( list, SIGNAL( clipSelected( Clip* ) ),
             this, SIGNAL( clipSelected( Clip* ) ) );
    connect( m_nav, SIGNAL( importRequired() ),
             this, SIGNAL( importRequired() ) );
    m_nav->pushViewController( list );
    setMinimumWidth( 280 );
}

MediaLibraryWidget::~MediaLibraryWidget()
{
    delete m_nav;
}
