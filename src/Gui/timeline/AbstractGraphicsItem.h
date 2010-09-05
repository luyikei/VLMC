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

#ifndef ABSTRACTGRAPHICSITEM_H
#define ABSTRACTGRAPHICSITEM_H

#include <QGraphicsItem>

class   GraphicsTrack;
class   TrackWorkflow;
class   TracksScene;
class   TracksView;

class AbstractGraphicsItem : public QObject, public QGraphicsItem
{
    Q_OBJECT

#if QT_VERSION >= 0x40600
    Q_INTERFACES( QGraphicsItem )
#endif

    public:
        enum From
        {
            BEGINNING,
            END
        };
        AbstractGraphicsItem();
        virtual ~AbstractGraphicsItem();

        /// Return the Type of the MediaItem (see http://doc.trolltech.com/4.5/qgraphicsitem.html#type)
        virtual int type() const = 0;
        /// Defines the outer bounds of the item as a rectangle
        virtual QRectF boundingRect() const;

        /// The item length can be expanded or shrinked by the user.
        virtual bool expandable() const = 0;

        /// The item can be moved by the user.
        virtual bool moveable() const = 0;

        /// Return a pointer to the TracksScene
        TracksScene* scene();

        /// Return the current track of the item
        qint32 trackNumber();

        /// Set the item's parent track
        void setTrack( GraphicsTrack* track );

        /// Return the item's parent track
        GraphicsTrack* track();

        /// Set the position of the item (in frames) for the x-axis.
        void setStartPos( qint64 position );

        /// Return the position of the item (in frames) for the x-axis.
        qint64 startPos();

        void    setColor( const QColor& color );
        QColor  itemColor();

        /**
         * \brief    Resize an item from its beginning or from its end.
         *
         *  \returns    A contextual info (depending on "from") to compute
         *              the new length. (Either the new beginning of the new length)
         */
        qint64      resize( qint64 size, qint64 newBegin, qint64 clipPos, From from = BEGINNING );

    protected:
        virtual void        hoverEnterEvent( QGraphicsSceneHoverEvent* event );
        virtual void        hoverMoveEvent( QGraphicsSceneHoverEvent* event );
        virtual void        mousePressEvent( QGraphicsSceneMouseEvent* event );
        virtual void        mouseReleaseEvent( QGraphicsSceneMouseEvent* event );
        /**
         * \brief Set the width of the item.
         * \param width Width in frames.
         */
        void setWidth( qint64 width );
        /**
         * \brief Set the height of the item.
         * \param height Height in pixels.
         */
        void setHeight( qint64 height );
        /**
         * \details Returns a pointer to the tracksView which contains the item,
         * or NULL if the item is not stored in a tracksView.
         */
        TracksView*         tracksView();
        /**
         *  \returns        True if the item has resize boundaries, false otherwise.
         */
        virtual bool        hasResizeBoundaries() const = 0;
        /**
         *  \brief          Return the begin boundaries for the item.
         */
        virtual qint64      maxBegin() const { return 0; }
        /**
         *  \brief          Return the end boundaries for the item.
         */
        virtual qint64      maxEnd() const { return -1; }

    protected:
        /// This pointer will be set when inserted in the tracksView.
        TracksView*         m_tracksView;
        QColor              m_itemColor;

    private:
        TrackWorkflow       *m_oldTrack;
        qint64              m_width;
        qint64              m_height;

    protected slots:
        /**
         * \brief Check if the position given as parameter could be taken as a resize request.
         * \return Returns True if the point is in a resize zone.
         */
        bool                resizeZone( const QPointF& position );

        friend class        TracksView;

};

#endif // ABSTRACTGRAPHICSITEM_H
