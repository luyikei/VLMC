/*****************************************************************************
 * MediaLibrary.cpp: VLMC media library's ui
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

#include "MediaLibrary.h"

#include "Clip.h"
#include "Library.h"
#include "Media.h"
#include "MediaCellView.h"
#include "MediaListView.h"
#include "StackViewController.h"
#include "ViewController.h"
#include "VlmcDebug.h"

#include <QUrl>
#include <QMimeData>

MediaLibrary::MediaLibrary(QWidget *parent) : QWidget(parent),
    m_ui( new Ui::MediaLibrary() )
{
    m_ui->setupUi( this );
    setAcceptDrops( true );

    StackViewController *nav = new StackViewController( m_ui->mediaListContainer );
    m_mediaListView = new MediaListView( nav, Library::getInstance() );
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

void
MediaLibrary::changeEvent( QEvent *e )
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
MediaLibrary::filterUpdated( const QString &filter )
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

MediaLibrary::Filter
MediaLibrary::currentFilter()
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
MediaLibrary::viewChanged( ViewController *view )
{
    MediaListView       *mlv = qobject_cast<MediaListView*>( view );

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

bool
MediaLibrary::filterByTags( const Clip *clip, const QString &filter )
{
    return ( clip->matchMetaTag( filter ) );
}

void
MediaLibrary::filterTypeChanged()
{
    filterUpdated( m_ui->filterInput->text() );
}

void
MediaLibrary::dragEnterEvent( QDragEnterEvent *event )
{
    if ( event->mimeData()->hasFormat( "text/uri-list" ) )
    {
        event->acceptProposedAction();
    }
    else
        event->ignore();
}

void
MediaLibrary::dragMoveEvent( QDragMoveEvent *event )
{
    event->acceptProposedAction();
}

void
MediaLibrary::dragLeaveEvent( QDragLeaveEvent *event )
{
   event->accept();
}

void
MediaLibrary::dropEvent( QDropEvent *event )
{
    const QList<QUrl>         &fileList = event->mimeData()->urls();

    if ( fileList.isEmpty() )
    {
        event->ignore();
        return;
    }

    Q_ASSERT( Library::getInstance() != NULL );

    foreach ( const QUrl &url, fileList )
    {
        const QString       &fileName = url.toLocalFile();

        if ( fileName.isEmpty() )
            continue;

        Media       *media = Library::getInstance()->addMedia( fileName );

        if ( media != NULL )
        {
            Clip*       clip = new Clip( media );
            media->setBaseClip( clip );
            Library::getInstance()->addClip( clip );
            event->accept();
        }
        else
            vlmcCritical() << "Clip already present in library or an error occurred while loading media:" << fileName;
    }
    event->accept();
}
