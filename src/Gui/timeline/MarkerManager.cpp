/*****************************************************************************
 * MarkerManager.cpp: Manager for markers
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "MarkerManager.h"

#include "Tools/VlmcDebug.h"

void
MarkerManager::addMarker( quint64 pos )
{
    if ( m_markers.contains( pos ) == false )
    {
        m_markers << pos;
        emit markerAdded( pos );
    }
    else
        vlmcCritical() << "Marker at" << pos << "has been already created.";
}

void
MarkerManager::moveMarker( quint64 from, quint64 to )
{
    auto it = m_markers.find( from );
    if ( it != m_markers.end() )
    {
        m_markers.erase( it );
        m_markers << to;
        emit markerMoved( from, to );
        return;
    }
    vlmcCritical() << "Marker at" << from << "doesn't exist.";
}

void
MarkerManager::removeMarker( quint64 pos )
{
    bool ret = m_markers.remove( pos );
    if ( ret == true )
        emit markerRemoved( pos );
    else
        vlmcCritical() << "Marker at" << pos << "doesn't exist.";
}

QVariant
MarkerManager::toVariant()
{
    QVariantList l;
    for ( auto pos : m_markers )
        l << pos;
    return l;
}

void
MarkerManager::fromVariant( const QVariant& variant )
{
    const auto& markers = variant.toList();
    for ( auto pos : markers )
        addMarker( pos.toULongLong() );
}
