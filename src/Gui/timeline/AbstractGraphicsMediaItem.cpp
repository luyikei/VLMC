/*****************************************************************************
 * AbstractGraphicsMediaItem.h: Base class for media representation
 *****************************************************************************
 * Copyright (C) 2008-2010 VideoLAN
 *
 * Authors: Ludovic Fauvet <etix@l0cal.com>
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

#include "AbstractGraphicsMediaItem.h"
#include "TracksView.h"
#include "TracksScene.h"
#include "TrackWorkflow.h"
#include "GraphicsTrack.h"

#include "Clip.h"
#include "ClipHelper.h"
#include "Commands.h"
#include "Media.h"

#include <QMenu>
#include <QColorDialog>
#include <QGraphicsSceneContextMenuEvent>

#include <QCoreApplication>
#include <QtDebug>

AbstractGraphicsMediaItem::AbstractGraphicsMediaItem( Clip* clip ) :
        m_oldTrack( NULL ),
        oldPosition( -1 ),
        m_tracksView( NULL ),
        m_group( NULL ),
        m_width( 0 ),
        m_height( 0 ),
        m_muted( false )
{
    m_clipHelper = new ClipHelper( clip );
    // Adjust the width
    setWidth( clip->length() );
    // Automatically adjust future changes
    connect( m_clipHelper, SIGNAL( lengthUpdated() ), this, SLOT( adjustLength() ) );
    connect( clip, SIGNAL( unloaded( Clip* ) ),
             this, SLOT( clipDestroyed( Clip* ) ), Qt::DirectConnection );
}

AbstractGraphicsMediaItem::AbstractGraphicsMediaItem( ClipHelper* ch ) :
        m_oldTrack( NULL ),
        oldPosition( -1 ),
        m_clipHelper( ch ),
        m_tracksView( NULL ),
        m_group( NULL ),
        m_width( 0 ),
        m_height( 0 ),
        m_muted( false )
{
    // Adjust the width
    setWidth( ch->length() );
    // Automatically adjust future changes
    connect( ch, SIGNAL( lengthUpdated() ), this, SLOT( adjustLength() ) );
    connect( ch->clip(), SIGNAL( unloaded( Clip* ) ),
             this, SLOT( clipDestroyed( Clip* ) ), Qt::DirectConnection );
}

AbstractGraphicsMediaItem::~AbstractGraphicsMediaItem()
{
    ungroup();
}

TracksScene* AbstractGraphicsMediaItem::scene()
{
    return qobject_cast<TracksScene*>( QGraphicsItem::scene() );
}

TracksView* AbstractGraphicsMediaItem::tracksView()
{
    return m_tracksView;
}

QRectF AbstractGraphicsMediaItem::boundingRect() const
{
    return QRectF( 0, 0, (qreal)m_width, (qreal)m_height );
}

void AbstractGraphicsMediaItem::setWidth( qint64 width )
{
    prepareGeometryChange();
    m_width = width;
}

void AbstractGraphicsMediaItem::setHeight( qint64 height )
{
    prepareGeometryChange();
    m_height = height;
}

qint32 AbstractGraphicsMediaItem::trackNumber()
{
    if ( parentItem() )
    {
        GraphicsTrack* graphicsTrack = qgraphicsitem_cast<GraphicsTrack*>( parentItem() );
        if ( graphicsTrack )
            return graphicsTrack->trackNumber();
    }
    return -1;
}

void AbstractGraphicsMediaItem::setTrack( GraphicsTrack* track )
{
    setParentItem( track );
}

GraphicsTrack* AbstractGraphicsMediaItem::track()
{
    return qgraphicsitem_cast<GraphicsTrack*>( parentItem() );
}

void AbstractGraphicsMediaItem::group( AbstractGraphicsMediaItem* item )
{
    Q_ASSERT( item );
    if ( m_group )
        ungroup();
    item->m_group = this;
    m_group = item;
}

void AbstractGraphicsMediaItem::ungroup()
{
    if ( !m_group ) return;
    m_group->m_group = NULL;
    m_group = NULL;
}

AbstractGraphicsMediaItem* AbstractGraphicsMediaItem::groupItem()
{
    return m_group;
}

void AbstractGraphicsMediaItem::contextMenuEvent( QGraphicsSceneContextMenuEvent* event )
{
    if ( !tracksView() )
        return;

    QMenu menu( tracksView() );

    QAction* removeAction = menu.addAction( "Remove" );
    QAction* muteAction = menu.addAction( "Mute" );
    muteAction->setCheckable( true );
    muteAction->setChecked( m_muted );

    QAction* linkAction = NULL;
    QAction* unlinkAction = NULL;

    if ( groupItem() )
        unlinkAction = menu.addAction( "Unlink" );
    else
    {
        QList<QGraphicsItem*> items = scene()->selectedItems();
        linkAction = menu.addAction( "Link" );

        if ( items.count() != 2 )
            linkAction->setEnabled( false );
    }

    menu.addSeparator();

    QAction* changeColorAction = menu.addAction( "Set color" );

    QAction* selectedAction = menu.exec( event->screenPos() );

    if ( !selectedAction )
        return;

    if ( selectedAction == removeAction )
        scene()->askRemoveSelectedItems();
    else if ( selectedAction == muteAction )
    {
        qint32      trackId = trackNumber();
        Q_ASSERT( trackId >= 0 );
        if ( ( m_muted = muteAction->isChecked() ) )
        {
            tracksView()->m_mainWorkflow->muteClip( uuid(),
                                                    trackId,
                                                    mediaType() );
        }
        else
        {
            tracksView()->m_mainWorkflow->unmuteClip( uuid(),
                                                    trackId,
                                                    mediaType() );
        }
    }
    else if ( selectedAction == linkAction )
    {
        QList<QGraphicsItem*> items = scene()->selectedItems();

        AbstractGraphicsMediaItem* item1;
        AbstractGraphicsMediaItem* item2;

        item1 = dynamic_cast<AbstractGraphicsMediaItem*>( items.at( 0 ) );
        item2 = dynamic_cast<AbstractGraphicsMediaItem*>( items.at( 1 ) );

        Q_ASSERT( item1 );
        Q_ASSERT( item2 );

        if ( item1 == this )
            item1 = item2;
        //From here, the item we click on is "this" and the item to group is "item1"

        if ( item1->mediaType() != mediaType() )
        {
            qint32      item1TrackId = item1->trackNumber();
            Q_ASSERT( item1TrackId >= 0 );
            item1->group( this );
            tracksView()->moveMediaItem( item1, item1TrackId , startPos() );
            track()->trackWorkflow()->moveClip( item1->clipHelper()->uuid(), startPos() );
        }
    }
    else if ( selectedAction == unlinkAction )
    {
        QList<QGraphicsItem*> items = scene()->selectedItems();

        AbstractGraphicsMediaItem* item;
        item = dynamic_cast<AbstractGraphicsMediaItem*>( items.at( 0 ) );

        item->ungroup();
    }
    else if ( selectedAction == changeColorAction )
    {
        m_itemColor = QColorDialog::getColor( m_itemColor, tracksView() );
        update();
    }

}

void AbstractGraphicsMediaItem::setStartPos( qint64 position )
{
    QGraphicsItem::setPos( (qreal)position, 0 );
}

qint64 AbstractGraphicsMediaItem::startPos()
{
    return qRound64( QGraphicsItem::pos().x() );
}

qint64  AbstractGraphicsMediaItem::resize( qint64 newSize, qint64 newBegin, qint64 clipPos,
                                           From from )
{
    Q_ASSERT( clipHelper() );

    if ( newSize < 1 )
        return 1;

    if ( clipHelper()->clip()->getMedia()->fileType() != Media::Image )
        if ( newSize > clipHelper()->clip()->end() )
            newSize = clipHelper()->clip()->end();

    //The from actually stands for the clip bound that stays still.
    if ( from == BEGINNING )
    {
        if ( m_clipHelper->clip()->getMedia()->fileType() != Media::Image )
        {
            if ( m_clipHelper->begin() + newSize > m_clipHelper->clip()->end() )
                return m_clipHelper->length();
        }
        setWidth( newSize );
        return newSize;
    }
    else
    {
        if ( m_clipHelper->clip()->getMedia()->fileType() != Media::Image )
        {
            if ( m_clipHelper->clip()->begin() > newBegin )
                return m_clipHelper->clip()->begin();
        }
        setWidth( newSize );
        setStartPos( clipPos );
        return newBegin;
    }
}

void AbstractGraphicsMediaItem::adjustLength()
{
    Q_ASSERT( m_clipHelper );
    setWidth( m_clipHelper->length() );
}

bool AbstractGraphicsMediaItem::resizeZone( const QPointF& position )
{
    // Get the current transformation of the view and invert it.
    QTransform transform = tracksView()->transform().inverted();
    // Map the RESIZE_ZONE distance from the view to the item coordinates.
    QLineF line = transform.map( QLineF( 0, 0, RESIZE_ZONE, 0 ) );

    if ( position.x() < line.x2() ||
         position.x() > ( boundingRect().width() - line.x2() ) )
    {
        return true;
    }
    return false;
}

QColor
AbstractGraphicsMediaItem::itemColor()
{
    return m_itemColor;
}

void
AbstractGraphicsMediaItem::setColor( const QColor &color )
{
    m_itemColor = color;
}

void
AbstractGraphicsMediaItem::clipDestroyed( Clip* clip )
{
    if ( m_tracksView != NULL )
        m_tracksView->removeClip( clip->uuid() );
}

ClipHelper*
AbstractGraphicsMediaItem::clipHelper()
{
    return m_clipHelper;
}

const QUuid&
AbstractGraphicsMediaItem::uuid() const
{
    return m_clipHelper->uuid();
}
