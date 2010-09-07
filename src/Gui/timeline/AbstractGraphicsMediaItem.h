/*****************************************************************************
 * AbstractGraphicsMediaItem.h: Base class for media representation
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

#ifndef ABSTRACTGRAPHICSMEDIAITEM_H
#define ABSTRACTGRAPHICSMEDIAITEM_H

class   ClipHelper;
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
    AbstractGraphicsMediaItem( ClipHelper* ch );
    virtual ~AbstractGraphicsMediaItem();

    /// Should return the unique uid of the contained media.
    virtual const QUuid& uuid() const;

    ClipHelper  *clipHelper();
    const ClipHelper*   clipHelper() const;

    virtual void        setEmphasized( bool value );
    virtual qint64      begin() const;
    virtual qint64      end() const;

protected:
    virtual void        contextMenuEvent( QGraphicsSceneContextMenuEvent* event );
    virtual void        hoverEnterEvent( QGraphicsSceneHoverEvent* event );
    virtual void        mousePressEvent( QGraphicsSceneMouseEvent* event );
    virtual bool        hasResizeBoundaries() const;
    virtual qint64      maxBegin() const;
    virtual qint64      maxEnd() const;

protected:
    ClipHelper*         m_clipHelper;

private slots:
    /**
     * \brief Adjust the length of the item according to the associated Clip.
     * \details This method should be called when the clip size change
     */
    void adjustLength();

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
