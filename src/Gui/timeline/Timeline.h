/*****************************************************************************
 * Timeline.h: Widget that handle the tracks
 *****************************************************************************
 * Copyright (C) 2008-2016 VideoLAN
 *
 * Authors: Ludovic Fauvet <etix@l0cal.com>
 *          Yikei Lu    <luyikei.qmltu@gmail.com>
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

#ifndef TIMELINE_H
#define TIMELINE_H

#include "Media/Clip.h"
#include "vlmc.h"
#include "Workflow/Types.h"

#include <QSharedPointer>

class MainWindow;
class QQuickView;
class Settings;
class MarkerManager;

/**
 * \brief Entry point of the timeline widget.
 */
class Timeline : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY( Timeline )
public:
    explicit Timeline( Settings* projectSettings, MainWindow* parent = 0 );
    virtual ~Timeline();

    QWidget*            container();

signals:
    void                markerAdded( quint64 pos );
    void                markerMoved( quint64 from, quint64 to );
    void                markerRemoved( quint64 pos );

public slots:
    void                addMarker( quint64 pos );
    void                moveMarker( quint64 from, quint64 to );
    void                removeMarker( quint64 pos );

private slots:
    void                preSave();
    void                postLoad();

protected:
    virtual void changeEvent( QEvent *e ) { Q_UNUSED( e ) }

private:
    QQuickView*         m_view;
    QWidget*            m_container;

    QSharedPointer<MarkerManager> m_markerManager;
    std::unique_ptr<Settings> m_settings;
};

#endif // TIMELINE_H
