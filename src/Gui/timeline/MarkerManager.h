/*****************************************************************************
 * MarkerManager.h: Manager for markers
 *****************************************************************************
 * Copyright (C) 2008-2017 VideoLAN
 *
 * Authors: Yikei Lu    <luyikei.qmltu@gmail.com>
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

#ifndef MARKERMANAGER_H
#define MARKERMANAGER_H

#include <QObject>
#include <QSet>
#include <QVariant>

class MarkerManager : public QObject
{
    Q_OBJECT
public:
    MarkerManager() = default;

    void                addMarker( quint64 pos );
    void                moveMarker( quint64 from, quint64 to );
    void                removeMarker( quint64 pos );

    QVariant            toVariant();
    void                fromVariant( const QVariant& variant );

signals:
    void                markerAdded( quint64 pos );
    void                markerMoved( quint64 from, quint64 to );
    void                markerRemoved( quint64 pos );

private:
    QSet<quint64>       m_markers;
};

#endif // MARKERMANAGER_H
