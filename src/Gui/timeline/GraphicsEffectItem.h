/*****************************************************************************
 * GraphicsEffectItem.h: Represent an effect in the timeline.
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

#ifndef GRAPHICSEFFECTITEM_H
#define GRAPHICSEFFECTITEM_H

#include "AbstractGraphicsItem.h"
#include "EffectsEngine.h"

class   QUuid;

class GraphicsEffectItem : public AbstractGraphicsItem
{
    public:
        enum { Type = UserType + 3 };
        GraphicsEffectItem( Effect *effect );
        GraphicsEffectItem( EffectHelper *helper );

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
        virtual void                triggerMove(TrackWorkflow *oldTrack, TrackWorkflow *newTrack,
                                                Workflow::Helper *helper, qint64 pos);
        virtual void                triggerResize( TrackWorkflow *tw, Workflow::Helper *helper,
                                           qint64 newBegin, qint64 newEnd, qint64 pos );
        virtual qint64              itemHeight() const;
        virtual qint32              zSelected() const;
        virtual qint32              zNotSelected() const;
    protected:
        virtual bool                hasResizeBoundaries() const;
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

    private:
        Effect                      *m_effect;
        EffectHelper                *m_effectHelper;
};

#endif // GRAPHICSEFFECTITEM_H
