/*****************************************************************************
 * RecentProjects: Holds a list of recent projects by monitoring project events
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "RecentProjects.h"
#include "Settings/Settings.h"

RecentProjects::RecentProjects( Settings* vlmcSettings, QObject *parent )
    : QObject(parent)
{
    m_recentsProjects = vlmcSettings->createVar( SettingValue::List, "private/RecentsProjects", QVariantList(),
                                                "", "", SettingValue::Private );
    connect( m_recentsProjects, &SettingValue::changed, this, &RecentProjects::updated );
}

QString
RecentProjects::mostRecentProjectFile()
{
    auto l = toVariant().toList();
    if ( l.size() == 0 )
        return QStringLiteral( "" );
    return l[0].toMap()["file"].toString();
}

const QVariant&
RecentProjects::toVariant() const
{
    return m_recentsProjects->get();
}

void
RecentProjects::remove( const QString &projectFile )
{
    QVariantList l = toVariant().toList();
    for ( int i = 0; i < l.count(); ++i )
    {
        if ( l[i].toMap()["file"].toString() == projectFile )
        {
            l.removeAt( i );
            --i;
        }
    }
    m_recentsProjects->set( l );
}

void
RecentProjects::projectLoaded( const QString& projectName, const QString& projectFile )
{
    QVariantList l = toVariant().toList();
    QVariantMap var {
        { "name", projectName },
        { "file", projectFile }
    };
    l.removeAll( var );
    l.insert( 0, var );
    m_recentsProjects->set( l );
}
