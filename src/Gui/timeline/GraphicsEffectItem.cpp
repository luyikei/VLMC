/*****************************************************************************
 * GraphicsEffectItem.cpp: Represent an effect in the timeline.
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

#include "GraphicsEffectItem.h"

#include "Commands.h"
#include "EffectHelper.h"
#include "EffectInstance.h"
#include "Timeline.h"
#include "TracksView.h"
#include "TrackWorkflow.h"

#include <QPainter>

#include <QtDebug>

GraphicsEffectItem::GraphicsEffectItem( Effect *effect ) :
        m_effect( effect ),
        m_effectHelper( NULL )
{
    setOpacity( 0.8 );
    m_effectHelper = new EffectHelper( effect->createInstance() );
    setWidth( m_effectHelper->length() );
}

GraphicsEffectItem::GraphicsEffectItem( EffectHelper *helper ) :
        m_effectHelper( helper )
{
    setWidth( m_effectHelper->length() );
    m_effect = helper->effectInstance()->effect();
    setOpacity( 0.8 );
}

const QUuid&
GraphicsEffectItem::uuid() const
{
    return m_effectHelper->uuid();
}

int
GraphicsEffectItem::type() const
{
    return Type;
}

bool
GraphicsEffectItem::expandable() const
{
    return true;
}

bool
GraphicsEffectItem::moveable() const
{
    return true;
}

bool
GraphicsEffectItem::hasResizeBoundaries() const
{
    return false;
}

Workflow::TrackType
GraphicsEffectItem::trackType() const
{
    return Workflow::VideoTrack;
}

void
GraphicsEffectItem::paintRect( QPainter* painter, const QStyleOptionGraphicsItem* option )
{
    QRectF drawRect;
    bool drawRound;

    // Disable the matrix transformations
    painter->setWorldMatrixEnabled( false );

    painter->setRenderHint( QPainter::Antialiasing );

    // Get the transformations required to map the text on the viewport
    QTransform viewPortTransform = Timeline::getInstance()->tracksView()->viewportTransform();

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

    gradient.setColorAt( 0, Qt::darkBlue );
    gradient.setColorAt( 1, Qt::blue );

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
        painter->setPen( Qt::yellow );
        painter->setBrush( Qt::NoBrush );
        mapped.adjust( 0, 0, 0, -1 );
        if ( drawRound )
            painter->drawRoundedRect( mapped, AbstractGraphicsItem::RounderRectRadius,
                                      AbstractGraphicsItem::RounderRectRadius);
        else
            painter->drawRect( mapped );
    }
}


void
GraphicsEffectItem::paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* )
{
    painter->save();
    paintRect( painter, option );
    painter->restore();

    painter->save();
    paintTitle( painter, option );
    painter->restore();
}

void
GraphicsEffectItem::paintTitle( QPainter* painter, const QStyleOptionGraphicsItem* option )
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
    QString text = m_effect->name();

    // Get the transformations required to map the text on the viewport
    QTransform viewPortTransform = Timeline::getInstance()->tracksView()->viewportTransform();
    // Do the transformation
    QRectF mapped = deviceTransform( viewPortTransform ).mapRect( boundingRect() );
    // Create an inner rect
    mapped.adjust( 2, 2, -2, -2 );

    painter->setPen( Qt::white );
    painter->drawText( mapped, Qt::AlignVCenter, fm.elidedText( text, Qt::ElideRight, mapped.width() ) );
}

EffectHelper*
GraphicsEffectItem::effectHelper()
{
    return m_effectHelper;
}


qint64
GraphicsEffectItem::begin() const
{
    return 0;
}

qint64
GraphicsEffectItem::end() const
{
    return -1;
}

Workflow::Helper*
GraphicsEffectItem::helper()
{
    return m_effectHelper;
}

void
GraphicsEffectItem::triggerMove( TrackWorkflow *oldTrack, TrackWorkflow *newTrack,
                                 Workflow::Helper *helper, qint64 pos )
{
    EffectHelper    *eh = qobject_cast<EffectHelper*>( helper );
    if ( eh == NULL )
        return ;
    Commands::trigger( new Commands::Effect::Move( eh, oldTrack, newTrack, pos ) );
}

void
GraphicsEffectItem::triggerResize( TrackWorkflow *, Workflow::Helper *helper,
                                   qint64 newBegin, qint64 newEnd, qint64 )
{
    EffectHelper    *eh = qobject_cast<EffectHelper*>( helper );
    if ( eh == NULL )
        return ;
    Commands::trigger( new Commands::Effect::Resize( eh, newBegin, newEnd ) );
}
