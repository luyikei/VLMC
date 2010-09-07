/*****************************************************************************
 * AbstractGraphiscItem.cpp: Represent an abstract item on the timeline
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

#include "AbstractGraphicsItem.h"

#include "GraphicsTrack.h"
#include "TracksView.h"
#include "TracksScene.h"

#include <QGraphicsSceneEvent>

#include <QtDebug>

AbstractGraphicsItem::AbstractGraphicsItem() :
        m_tracksView( NULL ),
        m_oldTrack( NULL ),
        m_width( 0 ),
        m_height( 0 ),
        m_group( NULL )
{
    setFlags( QGraphicsItem::ItemIsSelectable );
    setAcceptHoverEvents( true );
}

AbstractGraphicsItem::~AbstractGraphicsItem()
{
}

TracksScene*
AbstractGraphicsItem::scene()
{
    return qobject_cast<TracksScene*>( QGraphicsItem::scene() );
}


TracksView*
AbstractGraphicsItem::tracksView()
{
    return m_tracksView;
}

QRectF
AbstractGraphicsItem::boundingRect() const
{
    return QRectF( 0, 0, (qreal)m_width, (qreal)m_height );
}

void
AbstractGraphicsItem::setWidth( qint64 width )
{
    prepareGeometryChange();
    m_width = width;
}

void
AbstractGraphicsItem::setHeight( qint64 height )
{
    prepareGeometryChange();
    m_height = height;
}

qint32
AbstractGraphicsItem::trackNumber()
{
    if ( parentItem() )
    {
        GraphicsTrack* graphicsTrack = qgraphicsitem_cast<GraphicsTrack*>( parentItem() );
        if ( graphicsTrack )
            return graphicsTrack->trackNumber();
    }
    return -1;
}

void
AbstractGraphicsItem::setTrack( GraphicsTrack* track )
{
    setParentItem( track );
}

GraphicsTrack*
AbstractGraphicsItem::track()
{
    return qgraphicsitem_cast<GraphicsTrack*>( parentItem() );
}

void
AbstractGraphicsItem::setStartPos( qint64 position )
{
    QGraphicsItem::setPos( (qreal)position, 0 );
}

qint64
AbstractGraphicsItem::startPos()
{
    return qRound64( QGraphicsItem::pos().x() );
}

void
AbstractGraphicsItem::resize( qint64 newSize, qint64 newMovingBoundary,
                              qint64 currentStillBoundary, From from )
{
    if ( newSize < 1 )
    {
//        if ( from == END )
//            setStartPos( currentStillBoundary - 1 );
//        setWidth( 1 );
        return ;
    }

    qint64      maxSize = maxEnd() - maxBegin();
    //The "from" actually stands for the clip bound that stays still.
    if ( from == BEGINNING )
    {
        if ( hasResizeBoundaries() == true )
        {
            qint64      beginOffset = begin() - maxBegin();
            qint64      absoluteStartPos = currentStillBoundary - beginOffset;
            if ( absoluteStartPos + maxSize < newMovingBoundary )
            {
                setWidth( maxSize - beginOffset );
                return ;
            }
        }
        setWidth( newSize );
    }
    else
    {
        if ( hasResizeBoundaries() == true )
        {
            qint64      endOffset = maxEnd() - end();
            qint64      absoluteEnd = currentStillBoundary + endOffset;
            if ( newMovingBoundary < absoluteEnd - maxSize )
            {
//                qint64  size = end() - begin();
//                setWidth( size );
//                setStartPos( currentStillBoundary - size );
                return ;
            }
        }
        setWidth( newSize );
        setStartPos( newMovingBoundary );
    }
}

QColor
AbstractGraphicsItem::itemColor()
{
    return m_itemColor;
}

void
AbstractGraphicsItem::setColor( const QColor &color )
{
    m_itemColor = color;
}

void
AbstractGraphicsItem::hoverEnterEvent( QGraphicsSceneHoverEvent* event )
{
    setCursor( Qt::OpenHandCursor );
    QGraphicsItem::hoverEnterEvent( event );
}

void
AbstractGraphicsItem::hoverMoveEvent( QGraphicsSceneHoverEvent* event )
{
    if ( !tracksView() ) return;

    if ( tracksView()->tool() == TOOL_DEFAULT )
    {
        if ( resizeZone( event->pos() ) )
            setCursor( Qt::SizeHorCursor );
        else
            setCursor( Qt::OpenHandCursor );
    }
}

void
AbstractGraphicsItem::mousePressEvent( QGraphicsSceneMouseEvent* event )
{
    if ( !tracksView() ) return;

    if ( tracksView()->tool() == TOOL_DEFAULT )
    {
        if ( resizeZone( event->pos() ) )
            setCursor( Qt::SizeHorCursor );
        else
            setCursor( Qt::ClosedHandCursor );
    }
}

void
AbstractGraphicsItem::mouseReleaseEvent( QGraphicsSceneMouseEvent* )
{
    if ( !tracksView() ) return;

    if ( tracksView()->tool() == TOOL_DEFAULT )
        setCursor( Qt::OpenHandCursor );
}

bool
AbstractGraphicsItem::resizeZone( const QPointF& position )
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

AbstractGraphicsItem*
AbstractGraphicsItem::groupItem()
{
    return m_group;
}

void
AbstractGraphicsItem::group( AbstractGraphicsItem* item )
{
    Q_ASSERT( item );
    if ( m_group )
        ungroup();
    item->m_group = this;
    m_group = item;
}

void
AbstractGraphicsItem::ungroup()
{
    if ( m_group == NULL )
        return;
    m_group->m_group = NULL;
    m_group = NULL;
}

qint64
AbstractGraphicsItem::width() const
{
    return m_width;
}
