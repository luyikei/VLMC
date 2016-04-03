/*****************************************************************************
 * GraphicsTrack.cpp: Graphically represent a track in the timeline
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

#include <QList>

#include "Main/Core.h"
#include "Project/Project.h"
#include "TracksView.h"
#include "GraphicsTrack.h"
#include "Workflow/MainWorkflow.h"

GraphicsTrack::GraphicsTrack( Workflow::TrackType type, quint32 trackNumber,
                              QGraphicsItem *parent ) :
    QGraphicsWidget( parent ),
    m_emphasizer( nullptr )
{
    m_type = type;
    m_trackNumber = trackNumber;
    m_enabled = true;
    m_trackWorkflow = Core::instance()->workflow()->track( type, trackNumber );

    setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
    setContentsMargins( 0, 0, 0, 0 );
    setZValue( 1 );
}

void
GraphicsTrack::setHeight( int height )
{
    setPreferredHeight( height );
    adjustSize();
    updateGeometry();
}

int
GraphicsTrack::height()
{
    return preferredHeight();
}

void
GraphicsTrack::setTrackEnabled( bool enabled )
{
    if( enabled == m_enabled )
        return;
    m_enabled = enabled;

    if( enabled )
        Core::instance()->workflow()->unmuteTrack( m_trackNumber, m_type );
    else
        Core::instance()->workflow()->muteTrack( m_trackNumber, m_type );
}

bool
GraphicsTrack::isEnabled()
{
    return m_enabled;
}

quint32
GraphicsTrack::trackNumber()
{
    return m_trackNumber;
}

Workflow::TrackType
GraphicsTrack::mediaType()
{
    return m_type;
}

QList<AbstractGraphicsItem*>
GraphicsTrack::childs()
{
    QList<AbstractGraphicsItem*> list;
    QList<QGraphicsItem*> items = childItems();
    AbstractGraphicsItem* item;

    for ( int i = 0; i < items.count(); ++i )
    {
        item = dynamic_cast<AbstractGraphicsItem*>( items.at( i ) );
        if ( !item )
            continue;
        list.append( item );
    }
    return list;
}

TrackWorkflow*
GraphicsTrack::trackWorkflow()
{
    return m_trackWorkflow;
}

void
GraphicsTrack::setEmphasized( bool value )
{
    if ( m_emphasizer == nullptr )
        m_emphasizer = new EmphasizedTrackItem( this, maximumWidth(), preferredHeight() );
    if ( value == true )
        m_emphasizer->show();
    else
        m_emphasizer->hide();
}


EmphasizedTrackItem::EmphasizedTrackItem( GraphicsTrack *parent, qreal width, qreal height ) :
    QGraphicsItem( parent ),
    m_width( width ),
    m_height( height )
{
}

QRectF
EmphasizedTrackItem::boundingRect() const
{
    return QRectF( 0, 0, m_width, m_height );
}

void
EmphasizedTrackItem::paint(QPainter *painter, const QStyleOptionGraphicsItem* , QWidget* )
{
    painter->setBrush( QBrush( Qt::darkBlue ) );
    painter->drawRect( boundingRect() );
}
