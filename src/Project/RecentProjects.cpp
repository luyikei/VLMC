/*****************************************************************************
 * RecentProjects: Holds a list of recent projects by monitoring project events
 *****************************************************************************
 * Copyright (C) 2008-2014 VideoLAN
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

#include <QStringList>

#include "RecentProjects.h"

#include "Project/Project.h"
#include "Settings/Settings.h"
#include "Tools/VlmcDebug.h"

RecentProjects::RecentProjects( Settings* vlmcSettings, QObject *parent )
    : QObject(parent)
    , m_settings( vlmcSettings )
    , m_project( NULL )
{
    vlmcSettings->createVar( SettingValue::String, "private/RecentsProjects", "",
                                                "", "", SettingValue::Private );

    vlmcSettings->watchValue( "private/RecentsProjects",
                              this, SLOT( loadRecentProjects( QVariant ) ) );
}

void
RecentProjects::setProject( Project* projectManager )
{
    if ( m_project != NULL )
        disconnect( m_project, SIGNAL( projectLoaded( QString, QString ) ) );
    m_project = projectManager;
    connect( projectManager, SIGNAL( projectLoaded( QString, QString ) ),
             this, SLOT( projectLoaded( QString, QString ) ) );
}

void
RecentProjects::projectLoaded(const QString& projectName, const QString& projectFile)
{
    removeFromRecentProjects( projectName );
    RecentProject project;
    project.name = projectName;
    project.filePath = projectFile;
    m_recentsProjects.prepend( project );
    while ( m_recentsProjects.count() > 15 )
        m_recentsProjects.removeLast();

    Core::getInstance()->settings()->setValue( "private/RecentsProjects", flattenProjectList() );
}

const RecentProjects::List&
RecentProjects::list() const
{
    return m_recentsProjects;
}

QString
RecentProjects::flattenProjectList() const
{
    if ( m_recentsProjects.count() == 0 )
        return QString();
    QString     res;
    foreach ( RecentProject p, m_recentsProjects )
    {
        res += p.name + '#' + p.filePath + '#';
    }
    res.chop(1);
    return res;
}

void
RecentProjects::removeFromRecentProjects( const QString &projectPath )
{
    List::iterator  it = m_recentsProjects.begin();
    List::iterator  ite = m_recentsProjects.end();

    while ( it != ite )
    {
        if ( (*it).filePath == projectPath )
            it = m_recentsProjects.erase( it );
        else
            ++it;
    }
}

void
RecentProjects::remove( const QString& projectPath )
{
    removeFromRecentProjects( projectPath );
    Core::getInstance()->settings()->setValue( "private/RecentsProjects", flattenProjectList() );
}

void
RecentProjects::loadRecentProjects( const QVariant& recentProjects )
{
    // Only watch initial loading, we are now taking ownership of "private/RecentsProjects settings"
    disconnect( this, SLOT( loadRecentProjects( QVariant ) ) );

    const QStringList   recentProjectsList = recentProjects.toString().split( '#' );

    if ( recentProjectsList.count() == 0 )
        return ;

    QStringList::const_iterator     it = recentProjectsList.begin();
    QStringList::const_iterator     ite = recentProjectsList.end();
    while ( it != ite )
    {
        RecentProject project;
        project.name = *it;
        ++it;
        if ( it == ite )
        {
            vlmcWarning() << "Invalid flattened recent projects list.";
            return ;
        }
        project.filePath = *it;
        ++it;
        m_recentsProjects.append( project );
    }
}
