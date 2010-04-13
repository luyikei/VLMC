/*****************************************************************************
 * MediaLibraryWidget.h
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

#include "MediaListView.h"

#include "Clip.h"
#include "MediaCellView.h"
#include "MediaContainer.h"
#include "StackViewController.h"

MediaListView::MediaListView( StackViewController* nav, MediaContainer* mc ) :
        ListViewController( nav ),
        m_nav( nav ),
        m_mediaContainer( mc )
{
    connect( mc, SIGNAL( newClipLoaded(Clip*) ),
             this, SLOT( newClipLoaded( Clip* ) ) );
    connect( this, SIGNAL( clipRemoved( const QUuid& ) ),
             mc, SLOT( deleteClip( const QUuid& ) ) );
    connect( mc, SIGNAL( clipRemoved( const QUuid& ) ),
             this, SLOT( __clipRemoved( const QUuid& ) ) );
    connect( mc, SIGNAL( destroyed() ), this, SLOT( deleteLater() ) );
    foreach ( Clip* clip, mc->clips() )
        newClipLoaded( clip );
}

MediaListView::~MediaListView()
{
    foreach ( MediaCellView* cell, m_cells )
        delete cell;
    m_cells.clear();
}

void        MediaListView::newClipLoaded( Clip *clip )
{
    MediaCellView* cell = new MediaCellView( clip, m_container );

    connect( cell, SIGNAL ( cellSelected( QUuid ) ),
             this, SLOT ( cellSelection( QUuid ) ) );
    connect( cell, SIGNAL ( cellDeleted( const Clip* ) ),
             this, SLOT( removeClip( const Clip* ) ) );
    connect( cell, SIGNAL( arrowClicked( const QUuid& ) ),
             this, SLOT( showSubClips( const QUuid& ) ) );
    addCell( cell );
    m_cells.insert( clip->uuid(), cell );
    cellSelection( clip->uuid() );
}

void    MediaListView::cellSelection( const QUuid& uuid )
{
    if ( m_currentUuid == uuid )
        return;

    if ( m_cells.contains( uuid ) )
    {
        if ( !m_currentUuid.isNull() && m_cells.contains( m_currentUuid ) )
        {
            MediaCellView*  cell = m_cells.value( m_currentUuid );
            cell->setPalette( m_cells.value( uuid )->palette() );
        }
        QPalette p = m_cells.value( uuid )->palette();
        p.setColor( QPalette::Window, QColor( Qt::darkBlue ) );
        m_cells.value( uuid )->setPalette( p );
        m_currentUuid = uuid;
        emit clipSelected( m_mediaContainer->clip( uuid ) );
    }
}

void    MediaListView::removeClip( const Clip* clip )
{
    __clipRemoved( clip->uuid() );
    emit clipRemoved( clip->uuid() );
}

void
MediaListView::__clipRemoved( const QUuid &uuid )
{
    MediaCellView*  cell = m_cells.take( uuid );
    removeCell( cell );
    m_currentUuid = QUuid();
}

void
MediaListView::clear()
{
    foreach ( MediaCellView* cell, m_cells.values() )
        removeCell( cell );
    m_cells.clear();
}

void    MediaListView::showSubClips( const QUuid& uuid )
{
    Clip*   clip = m_mediaContainer->clip( uuid );
    m_nav->pushViewController( new MediaListView( m_nav, clip->getChilds() ) );
}
