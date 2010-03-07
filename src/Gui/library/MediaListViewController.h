/*****************************************************************************
 * MediaListViewController.h:
 *****************************************************************************
 * Copyright (C) 2008-2010 VideoLAN
 *
 * Authors: Thomas Boquet <thomas.boquet@gmail.com>
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

#ifndef MEDIALISTVIEWCONTROLLER_H
#define MEDIALISTVIEWCONTROLLER_H

#include "StackViewController.h"
#include "ListViewController.h"
#include "MediaCellView.h"
#include "ClipListViewController.h"
#include "Library.h"
#include "Media.h"

class MediaListViewController : public ListViewController
{
    Q_OBJECT

public:
    MediaListViewController( StackViewController* nav );
    virtual ~MediaListViewController();

private:
    StackViewController*    m_nav;
    QUuid                   m_currentUuid;
    QHash<QUuid, QWidget*>* m_cells;
    ClipListViewController* m_clipsListView;
    QUuid                   m_lastUuidClipListAsked;

public slots:
    //void        newMediaLoaded( Media* );
    void        newMediaLoaded( Media* media );
    void        cellSelection( const QUuid& uuid );
    void        mediaRemoved( const QUuid& uuid );
    void        updateCell( const Media* media );
    void        showClipList( const QUuid& uuid );
    void        newClipAdded( Clip* clip );
    void        clipSelection( const QUuid& uuid );

private slots:
    void        restoreContext();
signals:
    void        mediaSelected( Media* media );
    void        mediaDeleted( const QUuid& uuid );
    void        clipSelected( Clip* clip );

};
#endif // MEDIALISTVIEWCONTROLLER_H
