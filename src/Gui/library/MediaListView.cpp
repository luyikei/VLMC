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
#include "MediaContainer.h"

MediaListView::MediaListView( StackViewController* nav, MediaContainer* mc ) :
        ListViewController( nav ),
        m_nav( nav ),
        m_mediaContainer( mc )
{
    connect( mc, SIGNAL( newClipLoaded(Clip*) ),
             this, SLOT( newClipLoaded( Clip* ) ) );
    connect( this, SIGNAL( clipDeleted( const QUuid& ) ),
             mc, SLOT(removeClip( const QUuid& ) ) );
    foreach ( Clip* clip, mc->clips() )
        newClipLoaded( clip );
}

MediaListView::~MediaListView()
{
    foreach ( QWidget* cell, m_cells )
        delete cell;
    m_cells.clear();
}

void        MediaListView::newClipLoaded( Clip *clip )
{
    MediaCellView* cell = new MediaCellView( clip );

    connect( cell, SIGNAL ( cellSelected( QUuid ) ),
             this, SLOT ( cellSelection( QUuid ) ) );
    connect( cell, SIGNAL ( cellDeleted( const Clip* ) ),
             this, SLOT( clipRemoved( const Clip* ) ) );
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
            QWidget* cell = m_cells.value( m_currentUuid );
            cell->setPalette( m_cells.value( uuid )->palette() );
        }
        QPalette p = m_cells.value( uuid )->palette();
        p.setColor( QPalette::Window, QColor( Qt::darkBlue ) );
        m_cells.value( uuid )->setPalette( p );
        m_currentUuid = uuid;
        emit clipSelected( m_mediaContainer->clip( uuid ) );
    }
}

void    MediaListView::clipRemoved( const Clip* clip )
{
    QWidget* cell = m_cells.value( clip->uuid() );
    removeCell( cell );
    m_cells.remove( clip->uuid() );
    m_currentUuid = QUuid();
    emit clipDeleted( clip->uuid() );
    delete clip;
}

void
MediaListView::clear()
{
    foreach ( QWidget* cell, m_cells.values() )
        removeCell( cell );
    m_cells.clear();
}

void    MediaListView::showSubClips( const QUuid& uuid )
{
    Clip*   clip = m_mediaContainer->clip( uuid );
    m_nav->pushViewController( new MediaListView( m_nav, clip->getChilds() ) );
}
