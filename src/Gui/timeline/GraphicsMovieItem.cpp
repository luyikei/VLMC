/*****************************************************************************
 * GraphicsMovieItem.cpp: Represent a movie graphically in the timeline
 *****************************************************************************
 * Copyright (C) 2008-2016 VideoLAN
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

#include "Media/Clip.h"
#include "Workflow/ClipHelper.h"
#include "GraphicsMovieItem.h"
#include "Backend/ISource.h"
#include "Media/Media.h"
#include "TracksView.h"
#include "Timeline.h"

#include <QPainter>
#include <QLinearGradient>
#include <QTime>
#include <QFontMetrics>
#include <QGraphicsSceneMouseEvent>

GraphicsMovieItem::GraphicsMovieItem( Clip* clip ) :
        AbstractGraphicsMediaItem( clip )
{
    QTime length = QTime().addMSecs( clip->getMedia()->source()->length() );
    QString tooltip( tr( "<p style='white-space:pre'><b>Name:</b> %1"
                     "<br><b>Length:</b> %2" )
                     .arg( clip->getMedia()->fileName() )
                     .arg( length.toString("hh:mm:ss.zzz") ) );
    setToolTip( tooltip );
}

GraphicsMovieItem::GraphicsMovieItem( ClipHelper* ch ) :
        AbstractGraphicsMediaItem( ch )
{
    QTime length = QTime().addMSecs( ch->clip()->getMedia()->source()->length() );
    QString tooltip( tr( "<p style='white-space:pre'><b>Name:</b> %1"
                     "<br><b>Length:</b> %2" )
                     .arg( ch->clip()->getMedia()->fileName() )
                     .arg( length.toString("hh:mm:ss.zzz") ) );
    setToolTip( tooltip );
}

GraphicsMovieItem::~GraphicsMovieItem()
{
}

Workflow::TrackType GraphicsMovieItem::trackType() const
{
    return Workflow::VideoTrack;
}

void
GraphicsMovieItem::paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* )
{
    painter->save();
    paintRect( painter, option );
    painter->restore();

    painter->save();
    paintTitle( painter, option );
    painter->restore();
}

void
GraphicsMovieItem::paintRect( QPainter* painter, const QStyleOptionGraphicsItem* option )
{
    QRectF drawRect;
    bool drawRound;

    // Disable the matrix transformations
    painter->setWorldMatrixEnabled( false );

    painter->setRenderHint( QPainter::Antialiasing );

    // Get the transformations required to map the text on the viewport
    QTransform viewPortTransform = Timeline::instance()->tracksView()->viewportTransform();

    // Determine if a drawing optimization can be used
    if ( option->exposedRect.left() > AbstractGraphicsItem::RounderRectRadius &&
         option->exposedRect.right() < boundingRect().right() - AbstractGraphicsItem::RounderRectRadius )
    {
        // Optimized: paint only the exposed (horizontal) area
        drawRect = QRectF( option->exposedRect.left(),
                           boundingRect().top(),
                           option->exposedRect.right(),
                           boundingRect().bottom() );
        drawRound = false;
    }
    else
    {
        // Unoptimized: the item must be fully repaint
        drawRect = boundingRect();
        drawRound = true;
    }

    // Do the transformation
    QRectF mapped = deviceTransform( viewPortTransform ).mapRect( drawRect );

    QLinearGradient gradient( mapped.topLeft(), mapped.bottomLeft() );
    gradient.setColorAt( 0, QColor::fromRgb( 78, 78, 78 ) );
    gradient.setColorAt( 0.4, QColor::fromRgb( 72, 72, 72 ) );
    gradient.setColorAt( 0.4, QColor::fromRgb( 50, 50, 50 ) );
    gradient.setColorAt( 1, QColor::fromRgb( 45, 45, 45 ) );

    painter->setPen( Qt::NoPen );
    painter->setBrush( QBrush( gradient ) );

    if ( drawRound )
        painter->drawRoundedRect( mapped, AbstractGraphicsItem::RounderRectRadius,
                                  AbstractGraphicsItem::RounderRectRadius );
    else
        painter->drawRect( mapped );

    if ( itemColor().isValid() )
    {
        QRectF mediaColorRect = mapped.adjusted( 3, 2, -3, -2 );
        painter->setPen( QPen( itemColor(), 2 ) );
        painter->drawLine( mediaColorRect.topLeft(), mediaColorRect.topRight() );
    }

    if ( isSelected() )
    {
        setZValue( zSelected() );
        painter->setPen( Qt::yellow );
        painter->setBrush( Qt::NoBrush );
        mapped.adjust( 0, 0, 0, -1 );
        if ( drawRound )
            painter->drawRoundedRect( mapped, AbstractGraphicsItem::RounderRectRadius,
                                      AbstractGraphicsItem::RounderRectRadius);
        else
            painter->drawRect( mapped );
    }
    else
        setZValue( zNotSelected() );
}

void
GraphicsMovieItem::paintTitle( QPainter* painter, const QStyleOptionGraphicsItem* option )
{
    Q_UNUSED( option );

    // Disable the matrix transformations
    painter->setWorldMatrixEnabled( false );

    // Setup the font
    QFont f = painter->font();
    f.setPointSize( 8 );
    painter->setFont( f );

    // Initiate the font metrics calculation
    QFontMetrics fm( painter->font() );
    QString text = m_clipHelper->clip()->getMedia()->fileName();

    // Get the transformations required to map the text on the viewport
    QTransform viewPortTransform = Timeline::instance()->tracksView()->viewportTransform();
    // Do the transformation
    QRectF mapped = deviceTransform( viewPortTransform ).mapRect( boundingRect() );
    // Create an inner rect
    mapped.adjust( 2, 2, -2, -2 );

    painter->setPen( Qt::white );
    painter->drawText( mapped, Qt::AlignVCenter, fm.elidedText( text, Qt::ElideRight, mapped.width() ) );
}
