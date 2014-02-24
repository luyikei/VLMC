/*****************************************************************************
 * GraphicsCursorItem.cpp: Timeline's cursor
 *****************************************************************************
 * Copyright (C) 2008-2014 VideoLAN
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

#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QCursor>
#include <QPen>
#include <QRectF>
#include "GraphicsCursorItem.h"

GraphicsCursorItem::GraphicsCursorItem( const QPen& pen ) :
        m_pen( pen ), m_mouseDown( false )
{
    setFlags( QGraphicsItem::ItemIgnoresTransformations |
              QGraphicsItem::ItemIsMovable );
#if QT_VERSION >= 0x040600
    setFlag( QGraphicsItem::ItemSendsGeometryChanges );
#endif

    setCursor( QCursor( Qt::SizeHorCursor ) );
    setZValue( 100 );

    m_boundingRect = QRectF( -2, 0, 3, 0 );
}

QRectF
GraphicsCursorItem::boundingRect() const
{
    return m_boundingRect;
}

void
GraphicsCursorItem::paint( QPainter* painter, const QStyleOptionGraphicsItem*, QWidget* )
{
    painter->setPen( m_pen );
    painter->drawLine( 0, 0, 0, m_boundingRect.height() );
}

QVariant
GraphicsCursorItem::itemChange( GraphicsItemChange change, const QVariant& value )
{
    //Position is changing :
    if ( change == QGraphicsItem::ItemPositionChange )
    {
        // When the cursor is moving fast, the viewport buffer
        // is not correctly updated, forcing it now.
        scene()->update( pos().x(), pos().y(), m_boundingRect.width(), m_boundingRect.height() );

        // Keep the y axis in-place.
        qreal posX = value.toPointF().x();
        if ( posX < 0 ) posX = 0;
        return QPoint( ( int ) posX, ( int ) pos().y() );
    }
    //The position HAS changed, ie we released the slider, or setPos has been called.
    else if ( change == QGraphicsItem::ItemPositionHasChanged )
    {
        if ( m_mouseDown )
            emit cursorPositionChanged( ( qint64 ) pos().x() );
        emit cursorMoved( ( qint64 ) pos().x() );
    }
    return QGraphicsItem::itemChange( change, value );
}

void
GraphicsCursorItem::mousePressEvent( QGraphicsSceneMouseEvent* event )
{
    m_mouseDown = true;

    event->accept();
    QGraphicsItem::mousePressEvent( event );
}

void
GraphicsCursorItem::mouseReleaseEvent( QGraphicsSceneMouseEvent* event )
{
    m_mouseDown = false;

    event->accept();
    QGraphicsItem::mouseReleaseEvent( event );
}

void
GraphicsCursorItem::frameChanged( qint64 newFrame, Vlmc::FrameChangedReason reason )
{
    if ( reason != Vlmc::TimelineCursor )
    {
        setPos( newFrame, pos().y() );
    }
}

void
GraphicsCursorItem::setHeight( int height )
{
    prepareGeometryChange();
    m_boundingRect.setHeight( height );
}
