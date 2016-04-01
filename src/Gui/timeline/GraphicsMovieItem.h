/*****************************************************************************
 * GraphicsMovieItem.h: Represent a movie graphically in the timeline
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

#ifndef GRAPHICSMOVIEITEM_H
#define GRAPHICSMOVIEITEM_H

#include "AbstractGraphicsMediaItem.h"
#include "TracksView.h"

class   Clip;

/**
 * \brief Represents a video item.
 */
class GraphicsMovieItem : public AbstractGraphicsMediaItem
{
    Q_OBJECT
public:
    /**
     * \brief See http://doc.trolltech.com/4.5/qgraphicsitem.html#type
     */
    enum { Type = UserType + 1 };
    GraphicsMovieItem( Clip* clipHelper );
    /// The same CTOR, but this one won't create a new ClipHelper.
    GraphicsMovieItem( ClipHelper* clipHelper );
    virtual ~GraphicsMovieItem();

    virtual int type() const { return Type; }
    virtual bool expandable() const { return false; }
    virtual bool moveable() const { return true; }
    virtual Workflow::TrackType trackType() const;
    virtual void paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0 );

protected:
    /**
     * \brief Paint the item's rectangle.
     * \param painter Pointer to a QPainter.
     * \param option Painting options.
     */
    void                paintRect( QPainter* painter, const QStyleOptionGraphicsItem* option );
    /**
     * \brief Paint the item's title.
     * \param painter Pointer to a QPainter.
     * \param option Painting options.
     */
    void                paintTitle( QPainter* painter, const QStyleOptionGraphicsItem* option );
};
#endif // GRAPHICSMOVIEITEM_H
