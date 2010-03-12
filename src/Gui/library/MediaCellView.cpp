/*****************************************************************************
 * MediaCellView.cpp
 *****************************************************************************
 * Copyright (C) 2008-2010 VideoLAN
 *
 * Authors: Thomas Boquet <thomas.boquet@gmail.com>
 *          Hugo Beauzée-Luyssen <beauze.h@gmail.com>
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

#include "MediaCellView.h"
#include "ui_MediaCellView.h"

#include "Media.h"
#include "Library.h"
#include "ClipProperty.h"

#include <QTime>

MediaCellView::MediaCellView( Clip* clip, QWidget *parent ) :
        QWidget( parent ),
        m_ui( new Ui::MediaCellView ),
        m_clip( clip )
{
    m_ui->setupUi( this );
    setFocusPolicy( Qt::ClickFocus );
    setAutoFillBackground( true );
    connect( m_ui->delLabel, SIGNAL( clicked( QWidget*, QMouseEvent* ) ),
             this, SLOT( deleteButtonClicked( QWidget*, QMouseEvent* ) ) );
    connect( m_ui->arrow,
             SIGNAL( clicked( QWidget*, QMouseEvent* ) ),
             SLOT( arrowButtonClicked( QWidget*, QMouseEvent* ) ) );
    m_ui->clipCount->setText( QString::number( clip->getChilds()->count() ) );
    connect( clip->getChilds(), SIGNAL( newClipLoaded( Clip* ) ),
             this, SLOT( nbClipUpdated( Clip* ) ) );
    connect( clip->getChilds(), SIGNAL( clipRemoved( const Clip* ) ),
             this, SLOT( nbClipUpdated( const Clip* ) ) );
    if ( clip->getChilds()->count() == 0 )
    {
        m_ui->clipCount->hide();
        m_ui->clipCountLabel->hide();
        m_ui->arrow->hide();
    }
    if ( clip->getMedia()->isMetadataComputed() == false )
        setEnabled( false );
    connect( clip->getMedia(), SIGNAL( metaDataComputed(const Media*) ),
             this, SLOT( metadataUpdated( const Media*) ) );
    connect( clip->getMedia(), SIGNAL( snapshotComputed(const Media*) ),
             this, SLOT( snapshotUpdated(const Media*) ) );

    setThumbnail( clip->getMedia()->snapshot() );
    setTitle( clip->getMedia()->fileName() );
    setLength( clip->lengthSecond() * 1000 );
}

MediaCellView::~MediaCellView()
{
    delete m_ui;
}

void
MediaCellView::metadataUpdated( const Media *media )
{
    setLength( media->lengthMS() );
    setEnabled( true );
}

void
MediaCellView::snapshotUpdated( const Media *media )
{
    setThumbnail( media->snapshot() );
}

void MediaCellView::changeEvent( QEvent *e )
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

void            MediaCellView::setTitle( const QString& title )
{
    m_ui->title->setText( title );
}

void
MediaCellView::nbClipUpdated( Clip *clip )
{
    nbClipUpdated( const_cast<const Clip*>( clip ) );
}

void            MediaCellView::nbClipUpdated( const Clip *clip )
{
    quint32     nbClips = clip->getParent()->getChilds()->count();

    if ( nbClips == 0 )
    {
        m_ui->clipCount->hide();
        m_ui->clipCountLabel->hide();
        m_ui->arrow->hide();
        m_ui->clipCount->setText( "0" );
    }
    else
    {
        m_ui->clipCount->show();
        m_ui->clipCountLabel->show();
        m_ui->arrow->show();
        m_ui->clipCount->setText( QString::number( nbClips ) );
    }
}

void
MediaCellView::setThumbnail( const QPixmap &pixmap )
{
    m_ui->thumbnail->setScaledContents( false );
    m_ui->thumbnail->setPixmap( pixmap.scaled( 64, 64, Qt::KeepAspectRatio ) );
}

const QPixmap*  MediaCellView::getThumbnail() const
{
    return m_ui->thumbnail->pixmap();
}

QString  MediaCellView::title() const
{
    return m_ui->title->text();
}

void            MediaCellView::mouseDoubleClickEvent( QMouseEvent* event )
{
    if ( ( event->buttons() | Qt::LeftButton ) == Qt::LeftButton )
    {
        ClipProperty* cp = new ClipProperty( m_clip, this );
        cp->setModal( true );
        cp->exec();
        delete cp;
    }
}

void            MediaCellView::mousePressEvent( QMouseEvent* event )
{
    QWidget::mousePressEvent( event );

    if ( ( event->buttons() | Qt::LeftButton ) == Qt::LeftButton )
    {
        m_dragStartPos = event->pos();
        emit cellSelected( m_clip->uuid() );
    }
}

void    MediaCellView::mouseMoveEvent( QMouseEvent* event )
{
    if ( ( event->buttons() | Qt::LeftButton ) != Qt::LeftButton )
         return;

    if ( ( event->pos() - m_dragStartPos ).manhattanLength()
          < QApplication::startDragDistance() )
        return;

    QMimeData* mimeData = new QMimeData;
    mimeData->setData( "vlmc/uuid", m_clip->fullId().toAscii() );
    QDrag* drag = new QDrag( this );
    drag->setMimeData( mimeData );
    const Media*  parent = m_clip->getMedia();
    drag->setPixmap( parent->snapshot().scaled( 100, 100, Qt::KeepAspectRatio ) );
    drag->exec( Qt::CopyAction | Qt::MoveAction, Qt::CopyAction );
}

void        MediaCellView::deleteButtonClicked( QWidget*, QMouseEvent* )
{
    emit cellDeleted( m_clip );
}

void        MediaCellView::arrowButtonClicked( QWidget*, QMouseEvent* )
{
    emit arrowClicked( m_clip->uuid() );
}

void        MediaCellView::setLength( qint64 length, bool mSecs )
{
    QTime   duration;
    if ( mSecs )
        duration = duration.addMSecs( length );
    else
        duration = duration.addSecs( length );
    m_ui->length->setText( duration.toString( "hh:mm:ss" ) );
}

const QUuid&
MediaCellView::uuid() const
{
    return m_clip->uuid();
}
