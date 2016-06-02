/*****************************************************************************
 * MediaLibrary.cpp: VLMC media library's ui
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "MediaLibraryView.h"

#include "Project/Project.h"
#include "Media/Clip.h"
#include "Library/Library.h"
#include "Main/Core.h"
#include "Media/Media.h"
#include "MediaCellView.h"
#include "MediaListView.h"
#include "StackViewController.h"
#include "ViewController.h"
#include "Tools/VlmcDebug.h"

#include <QUrl>
#include <QMimeData>

MediaLibraryView::MediaLibraryView(QWidget *parent) : QWidget(parent),
    m_ui( new Ui::MediaLibraryView() )
{
    m_ui->setupUi( this );
    setAcceptDrops( true );

    StackViewController *nav = new StackViewController( m_ui->mediaListContainer );
    m_mediaListView = new MediaListView( nav );
    nav->pushViewController( m_mediaListView );

    connect( m_ui->importButton, SIGNAL( clicked() ),
             this, SIGNAL( importRequired() ) );
    connect( m_mediaListView, SIGNAL( clipSelected( Clip* ) ),
             this, SIGNAL( clipSelected( Clip* ) ) );
    connect( m_ui->filterInput, SIGNAL( textChanged( const QString& ) ),
             this, SLOT( filterUpdated( const QString& ) ) );
    connect( nav, SIGNAL( viewChanged( ViewController* ) ),
             this, SLOT( viewChanged( ViewController* ) ) );
    connect( m_ui->filterType, SIGNAL( currentIndexChanged( int ) ),
             this, SLOT( filterTypeChanged() ) );
}

MediaLibraryView::~MediaLibraryView()
{
    delete m_ui;
}

void
MediaLibraryView::changeEvent( QEvent *e )
{
    QWidget::changeEvent( e );
    switch ( e->type() )
    {
        case QEvent::LanguageChange:
            m_ui->retranslateUi( this );
            break;
        default:
            break;
    }
}

void
MediaLibraryView::filterUpdated( const QString &filter )
{
    const MediaListView::MediaList              &medias = m_mediaListView->mediaList();
    MediaListView::MediaList::const_iterator    it = medias.begin();
    MediaListView::MediaList::const_iterator    ite = medias.end();

    while ( it != ite )
    {
        MediaCellView  *mcv = it.value();

        mcv->setVisible( currentFilter()( mcv->clip(), filter ) );
        ++it;
    }
}

MediaLibraryView::Filter
MediaLibraryView::currentFilter()
{
    switch ( m_ui->filterType->currentIndex() )
    {
        case 0:
            return &filterByName;
        case 1:
            return &filterByTags;
        default:
            return &filterByName;
    }
}

void
MediaLibraryView::viewChanged( ViewController *view )
{
    MediaListView       *mlv = qobject_cast<MediaListView*>( view );

    if ( mlv == nullptr )
        return ;

    m_mediaListView = mlv;
    //Force an update as the media has changed
    filterUpdated( m_ui->filterInput->text() );
}

bool
MediaLibraryView::filterByName( const Clip *clip, const QString &filter )
{
    return ( clip->media()->fileName().contains( filter, Qt::CaseInsensitive ) );
}

bool
MediaLibraryView::filterByTags( const Clip *clip, const QString &filter )
{
    return ( clip->matchMetaTag( filter ) );
}

void
MediaLibraryView::filterTypeChanged()
{
    filterUpdated( m_ui->filterInput->text() );
}

void
MediaLibraryView::dragEnterEvent( QDragEnterEvent *event )
{
    if ( event->mimeData()->hasFormat( "text/uri-list" ) )
    {
        event->acceptProposedAction();
    }
    else
        event->ignore();
}

void
MediaLibraryView::dragMoveEvent( QDragMoveEvent *event )
{
    event->acceptProposedAction();
}

void
MediaLibraryView::dragLeaveEvent( QDragLeaveEvent *event )
{
   event->accept();
}

void
MediaLibraryView::dropEvent( QDropEvent *event )
{
    const QList<QUrl>         &fileList = event->mimeData()->urls();

    if ( fileList.isEmpty() )
    {
        event->ignore();
        return;
    }

    foreach ( const QUrl &url, fileList )
    {
        const QString       &fileName = url.toLocalFile();

        if ( fileName.isEmpty() )
            continue;

        Media       *media = Core::instance()->library()->addMedia( fileName );

        if ( media != nullptr )
        {
            Clip*       clip = new Clip( media );
            media->setBaseClip( clip );
            Core::instance()->library()->addClip( clip );
            event->accept();
        }
        else
            vlmcCritical() << "Clip already present in library or an error occurred while loading media:" << fileName;
    }
    event->accept();
}
