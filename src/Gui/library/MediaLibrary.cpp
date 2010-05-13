/*****************************************************************************
 * MediaLibrary.cpp: VLMC media library's ui
 *****************************************************************************
 * Copyright (C) 2008-2010 VideoLAN
 *
 * Authors: Hugo Beauzée-Luyssen <beauze.h@gmail.com>
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

#include "MediaLibrary.h"

#include "Clip.h"
#include "Library.h"
#include "Media.h"
#include "MediaCellView.h"
#include "MediaListView.h"
#include "StackViewController.h"
#include "ViewController.h"

#include <QtDebug>

MediaLibrary::MediaLibrary(QWidget *parent) : QWidget(parent),
    m_ui( new Ui::MediaLibrary() )
{
    m_ui->setupUi( this );
    StackViewController *nav = new StackViewController( m_ui->mediaListContainer );
    m_mediaListView = new MediaListView( nav, Library::getInstance() );
    nav->pushViewController( m_mediaListView );

    connect( m_ui->importButton, SIGNAL( clicked() ),
             this, SIGNAL( importRequired() ) );
    connect( m_mediaListView, SIGNAL( clipSelected( Clip* ) ),
             this, SIGNAL( clipSelected( Clip* ) ) );
    connect( m_ui->filterInput, SIGNAL( textChanged( const QString& ) ),
             this, SLOT( filterUpdated( const QString& ) ) );
    connect( m_ui->clearButton, SIGNAL( clicked() ),
             this, SLOT( clearFilter() ) );
    connect( nav, SIGNAL( viewChanged( ViewController* ) ),
             this, SLOT( viewChanged( ViewController* ) ) );
}

void
MediaLibrary::filterUpdated( const QString &filter )
{
    const MediaListView::MediaList  &medias = m_mediaListView->mediaList();
    MediaListView::MediaList::const_iterator        it = medias.begin();
    MediaListView::MediaList::const_iterator        ite = medias.end();

    while ( it != ite )
    {
        MediaCellView  *mcv = it.value();

        mcv->setVisible( currentFilter()( mcv->clip(), filter ) );
        ++it;
    }
}

void
MediaLibrary::clearFilter()
{
    m_ui->filterInput->setText( "" );
}

MediaLibrary::Filter
MediaLibrary::currentFilter()
{
    return &filterByName;
}

void
MediaLibrary::viewChanged( ViewController *view )
{
    MediaListView   *mlv = qobject_cast<MediaListView*>( view );

    if ( mlv == NULL )
        return ;
    m_mediaListView = mlv;
    //Force an update as the media has changed
    filterUpdated( m_ui->filterInput->text() );
}

bool
MediaLibrary::filterByName( const Clip *clip, const QString &filter )
{
    return ( clip->getMedia()->fileName().contains( filter, Qt::CaseInsensitive ) );
}
