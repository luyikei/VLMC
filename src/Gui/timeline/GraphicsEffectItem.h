/*****************************************************************************
 * GraphicsEffectItem.h: Represent an effect in the timeline.
 *****************************************************************************
 * Copyright (C) 2008-2016 VideoLAN
 *
 * Authors: Hugo Beauzée-Luyssen <hugo@beauzee.fr>
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

#ifndef GRAPHICSEFFECTITEM_H
#define GRAPHICSEFFECTITEM_H

#include "AbstractGraphicsItem.h"

class   EffectHelper;
class   AbstractGraphicsMediaItem;

namespace Backend
{
class IFilterInfo;
}

class GraphicsEffectItem : public AbstractGraphicsItem
{
    Q_OBJECT

    public:
        enum { Type = UserType + 3 };
        GraphicsEffectItem( EffectHelper *helper );

        virtual ~GraphicsEffectItem();

        virtual const QUuid&        uuid() const;
        virtual int                 type() const;
        virtual bool                expandable() const;
        virtual bool                moveable() const;
        virtual void                paint( QPainter *painter, const QStyleOptionGraphicsItem *option,
                                           QWidget *widget );
        virtual Workflow::TrackType trackType() const;
        EffectHelper*               effectHelper();
        virtual qint64              begin() const;
        virtual qint64              end() const;
        virtual qint64              maxBegin() const;
        virtual qint64              maxEnd() const;
        virtual Workflow::Helper    *helper();
        virtual void                triggerMove( TrackWorkflow* target, qint64 startPos );
        virtual void                triggerMove( Backend::IService* target, qint64 startPos );
        virtual void                triggerResize( TrackWorkflow* target, Workflow::Helper *helper,
                                           qint64 newBegin, qint64 newEnd, qint64 pos );
        virtual qint64              itemHeight() const;
        virtual qint32              zSelected() const;
        virtual qint32              zNotSelected() const;
        void                        setContainer( AbstractGraphicsMediaItem *item );
        const AbstractGraphicsMediaItem  *container() const;
        virtual void                setStartPos( qint64 position );

    protected:
        virtual bool                hasResizeBoundaries() const;
        virtual void                contextMenuEvent( QGraphicsSceneContextMenuEvent *event );
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
    private slots:
        void                containerMoved( qint64 pos );

    private:
        Backend::IFilterInfo        *m_effect;
        EffectHelper                *m_effectHelper;
        AbstractGraphicsMediaItem   *m_container;
};

#endif // GRAPHICSEFFECTITEM_H
