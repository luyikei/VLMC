/*****************************************************************************
 * GraphicsTrack.h: Graphically represent a track in the timeline
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

#ifndef GRAPHICSTRACK_H
#define GRAPHICSTRACK_H

#include <QGraphicsWidget>
#include <QList>
#include "Types.h"

class   AbstractGraphicsMediaItem;
class   TrackWorkflow;

class GraphicsTrack : public QGraphicsWidget
{
    Q_OBJECT

public:
    enum { Type = UserType + 2 };

    GraphicsTrack( Workflow::TrackType type, quint32 trackNumber,
                   QGraphicsItem *parent = 0 );

    void                setHeight( int height );
    int                 height();
    void                setTrackEnabled( bool enabled );
    bool                isEnabled();
    quint32             trackNumber();
    Workflow::TrackType mediaType();
    virtual int         type() const { return Type; }
    TrackWorkflow       *trackWorkflow();
    void                setEmphasized( bool value );

    QList<AbstractGraphicsMediaItem*> childs();

private:
    Workflow::TrackType m_type;
    quint32             m_trackNumber;
    bool                m_enabled;
    TrackWorkflow       *m_trackWorkflow;
    QGraphicsItem       *m_emphasizer;
};

class   EmphasizedTrackItem : public QGraphicsItem
{
    public:
        EmphasizedTrackItem( GraphicsTrack *parent, qreal width, qreal height );
        QRectF  boundingRect() const;
        void    paint( QPainter*, const QStyleOptionGraphicsItem*, QWidget* );
    private:
        qreal   m_width;
        qreal   m_height;
};

#endif // GRAPHICSTRACK_H
