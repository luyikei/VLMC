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
    connect( mc, SIGNAL( newMediaLoaded( Media* ) ),
             this, SLOT( newMediaLoaded( Media* ) ) );
    m_cells = new QHash<QUuid, QWidget*>();
    connect( m_nav, SIGNAL( previousButtonPushed() ), this, SLOT( restoreContext() ) );
}

MediaListViewController::~MediaListViewController()
{
    delete m_cells;
}

void        MediaListViewController::newMediaLoaded( Media* media )
{
    MediaCellView* cell = new MediaCellView( media->baseClip() );

    connect( cell, SIGNAL ( cellSelected( QUuid ) ),
             this, SLOT ( cellSelection( QUuid ) ) );
    connect( cell, SIGNAL ( cellDeleted( QUuid ) ),
             this, SIGNAL( clipDeleted( QUuid ) ) );
    connect( cell, SIGNAL( arrowClicked( const QUuid& ) ),
             this, SLOT( showClipList( const QUuid& ) ) );
    connect( media, SIGNAL( clipAdded( Clip* ) ),
             this, SLOT( newClipAdded(Clip*) ) );
    addCell(cell);
    m_cells->insert( media->baseClip()->uuid(), cell );
}

void    MediaListViewController::cellSelection( const QUuid& uuid )
{
    if ( m_currentUuid == uuid )
        return;

    if ( m_cells->contains( uuid ) )
    {
        if ( !m_currentUuid.isNull() && m_cells->contains( m_currentUuid ) )
        {
            QWidget* cell = m_cells->value( m_currentUuid );
            cell->setPalette( m_cells->value( uuid )->palette() );
        }
        QPalette p = m_cells->value( uuid )->palette();
        p.setColor( QPalette::Window, QColor( Qt::darkBlue ) );
        m_cells->value( uuid )->setPalette( p );
        m_currentUuid = uuid;
        emit clipSelected( m_mediaContainer->clip( uuid ) );
    }
}

void    MediaListViewController::mediaRemoved( const QUuid& uuid )
{
    QWidget* cell = m_cells->value( uuid );
    removeCell( cell );
    m_cells->remove( uuid );
    m_currentUuid = QUuid();
    emit clipDeleted( uuid );
}

void
MediaListViewController::clear()
{
    foreach ( QWidget* cell, m_cells->values() )
        removeCell( cell );
    m_cells->clear();
}

void    MediaListViewController::showClipList( const QUuid& uuid )
{
    if ( !m_cells->contains( uuid ) )
        return ;
    if ( m_mediaContainer->media( uuid ) == NULL ||
         m_mediaContainer->media( uuid )-> clipsCount() == 0 )
        return ;
    m_lastUuidClipListAsked = uuid;
    m_clipsListView = new MediaListViewController( m_nav, Library::getInstance() );
    m_clipsListView->newMediaLoaded( m_mediaContainer->media( uuid ) );
    connect( m_clipsListView, SIGNAL( clipSelected( const QUuid& ) ),
            this, SLOT( clipSelection( const QUuid& ) ) );
    m_nav->pushViewController( m_clipsListView );
}

void    MediaListViewController::newClipAdded( Clip* clip )
{
    if ( clip->getParent() == 0 )
        return ;
    const QUuid& uuid = clip->getParent()->baseClip()->uuid();

    if ( m_cells->contains( uuid ) )
    {
        MediaCellView*  cell = qobject_cast<MediaCellView*>( m_cells->value( uuid, 0 ) );
        if ( cell != 0 )
        {
            cell->incrementClipCount();
        }
    }
}

void
MediaListViewController::addClip( Clip *clip )
{
    newClipAdded( clip );
}

void    MediaListViewController::restoreContext()
{
    MediaCellView*  cell = qobject_cast<MediaCellView*>( m_cells->value( m_lastUuidClipListAsked, 0 ) );
    if ( cell != 0 )
    {
        qDebug() << "FIXME: Update clip count";
    }
    delete m_clipsListView;
}
