/*****************************************************************************
 * AbstractGraphicsMediaItem.h: Base class for media representation
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

#ifndef ABSTRACTGRAPHICSMEDIAITEM_H
#define ABSTRACTGRAPHICSMEDIAITEM_H

#include "AbstractGraphicsItem.h"

#include <QUuid>

#define RESIZE_ZONE 7

class   Clip;
class   TracksView;
class   TrackWorkflow;

/**
 * \brief Base class for Audio/Video items.
 */
class AbstractGraphicsMediaItem : public AbstractGraphicsItem
{
    Q_OBJECT

public:
    AbstractGraphicsMediaItem( Clip* clip );
    virtual ~AbstractGraphicsMediaItem();

    /// Should return the unique uid of the contained media.
    virtual const QUuid& uuid() const;

    Clip  *clip();
    const Clip*   clip() const;

    virtual void        setEmphasized( bool value );
    virtual qint64      begin() const;
    virtual qint64      end() const;

    virtual Workflow::Helper    *helper();
    virtual void        triggerMove( EffectUser *target, qint64 startPos );
    virtual void        triggerResize( EffectUser *tw, Workflow::Helper *helper,
                                       qint64 newBegin, qint64 newEnd, qint64 pos );
    virtual qint64              itemHeight() const;
    virtual qint32      zSelected() const;
    virtual qint32      zNotSelected() const;
    virtual void        setStartPos( qint64 position );

protected:
    virtual void        contextMenuEvent( QGraphicsSceneContextMenuEvent* event );
    virtual void        hoverEnterEvent( QGraphicsSceneHoverEvent* event );
    virtual void        mousePressEvent( QGraphicsSceneMouseEvent* event );
    virtual bool        hasResizeBoundaries() const;
    virtual qint64      maxBegin() const;
    virtual qint64      maxEnd() const;

protected:
    Clip*               m_clip;

private slots:
    void    clipDestroyed( Clip* clip );

private:
    bool    m_muted;

signals:
    /**
     * \brief Emitted when the item detect a cut request.
     * \param self A pointer to the sender.
     * \param frame Frame's number where the cut takes place.
     */
    void                split( AbstractGraphicsMediaItem* self, qint64 frame );
};

#endif // ABSTRACTGRAPHICSMEDIAITEM_H
