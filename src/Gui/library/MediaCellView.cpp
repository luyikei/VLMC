/*****************************************************************************
 * MediaCellView.cpp
 *****************************************************************************
 * Copyright (C) 2008-2010 VideoLAN
 *
 * Authors: Thomas Boquet <thomas.boquet@gmail.com>
 * Authors: Clement CHAVANCE <chavance.c@gmail.com>
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
#include "Library.h"
#include "ClipProperty.h"

#include <QTime>

#include <QtDebug>

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
    if ( clip->isRootClip() == true )
    {
        connect( m_ui->arrow,
                 SIGNAL( clicked( QWidget*, QMouseEvent* ) ),
                 SLOT( arrowButtonClicked( QWidget*, QMouseEvent* ) ) );
        m_ui->clipCount->setText( QString::number( clip->getParent()->clipsCount() ) );
        connect( clip->getParent(), SIGNAL( clipAdded(Clip*) ),
                 this, SLOT( nbClipUpdated( Clip* ) ) );
        connect( clip->getParent(), SIGNAL( clipRemoved( Clip* ) ),
                 this, SLOT( nbClipUpdated( Clip* ) ) );
    }
    else
    {
        m_ui->clipCount->hide();
        m_ui->clipCountLabel->hide();
        m_ui->arrow->hide();
        disconnect( m_ui->arrow,
                    SIGNAL( clicked( QWidget*, QMouseEvent* ) ), this,
                    SLOT( arrowButtonClicked( QWidget*, QMouseEvent* ) ) );
    }
    if ( clip->getParent()->isMetadataComputed() == false )
        setEnabled( false );
    connect( clip->getParent(), SIGNAL( metaDataComputed(const Media*) ),
             this, SLOT( metadataUpdated( const Media*) ) );
    connect( clip->getParent(), SIGNAL( snapshotComputed(const Media*) ),
             this, SLOT( snapshotUpdated(const Media*) ) );

    setThumbnail( clip->getParent()->snapshot() );
    setTitle( clip->getParent()->fileName() );
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

void            MediaCellView::nbClipUpdated( Clip *clip )
{
    m_ui->clipCount->setText( QString::number( clip->getParent()->clipsCount() ) );
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
    //FIXME the second argument is a media UUID instead of a Clip
    // and this is not logical... but it works.
    mimeData->setData( "vlmc/uuid", m_clip->uuid().toString().toAscii() );
    QDrag* drag = new QDrag( this );
    drag->setMimeData( mimeData );
    const Media*  parent = m_clip->getParent();
    drag->setPixmap( parent->snapshot().scaled( 100, 100, Qt::KeepAspectRatio ) );
    drag->exec( Qt::CopyAction | Qt::MoveAction, Qt::CopyAction );
}

void        MediaCellView::deleteButtonClicked( QWidget*, QMouseEvent* )
{
    emit cellDeleted( m_clip->uuid() );
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

void
MediaCellView::containsClip()
{

}

const QUuid&
MediaCellView::uuid() const
{
    return m_clip->uuid();
}
