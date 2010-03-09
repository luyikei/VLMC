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

#include "MediaListViewController.h"

#include "Clip.h"
#include "MediaContainer.h"

MediaListViewController::MediaListViewController( StackViewController* nav, MediaContainer* mc ) :
        ListViewController( nav ),
        m_nav( nav ),
        m_clipsListView( 0 ),
        m_mediaContainer( mc )
{
    connect( mc, SIGNAL( newClipLoaded(Clip*) ),
             this, SLOT( newClipLoaded( Clip* ) ) );
    foreach ( Clip* clip, mc->clips() )
        newClipLoaded( clip );
    connect( m_nav, SIGNAL( previousButtonPushed() ),
             this, SLOT( restoreContext() ) );
}

MediaListViewController::~MediaListViewController()
{
    foreach ( QWidget* cell, m_cells )
        delete cell;
    m_cells.clear();
}

void        MediaListViewController::newClipLoaded( Clip *clip )
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
}

void    MediaListViewController::cellSelection( const QUuid& uuid )
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

void    MediaListViewController::clipRemoved( const Clip* clip )
{
    QWidget* cell = m_cells.value( clip->uuid() );
    removeCell( cell );
    m_cells.remove( clip->uuid() );
    m_currentUuid = QUuid();
    m_mediaContainer->removeClip( clip );
    emit clipDeleted( clip->uuid() );
    delete clip;
}

void
MediaListViewController::clear()
{
    foreach ( QWidget* cell, m_cells.values() )
        removeCell( cell );
    m_cells.clear();
}

void    MediaListViewController::showSubClips( const QUuid& uuid )
{
    Clip*   clip = m_mediaContainer->clip( uuid );
    m_clipsListView = new MediaListViewController( m_nav, clip->getChilds() );
    m_nav->pushViewController( m_clipsListView );
}

void    MediaListViewController::restoreContext()
{
    delete m_clipsListView;
    m_currentUuid = QUuid();
}
