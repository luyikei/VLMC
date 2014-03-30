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

#ifndef RECENTPROJECTS_H
#define RECENTPROJECTS_H

#include <QObject>

class   Project;
class   Settings;

class RecentProjects : public QObject
{
    Q_OBJECT

    public:
        struct RecentProject
        {
            QString name;
            QString filePath;
        };
        typedef QList<RecentProject>      List;


        explicit RecentProjects(Settings* vlmcSettings, QObject *parent = 0 );

        void            setProject(Project* projectManager );
        void            remove( const QString& projectFile );
        const List&     list() const;

    private:
        void            removeFromRecentProjects( const QString &projectPath );
        QString         flattenProjectList() const;


    public slots:
        void            projectLoaded( const QString& projectName, const QString& projectFile );

    private slots:
        void            loadRecentProjects(const QVariant& recentProjects);

    private:
        Settings*       m_settings;
        Project*        m_project;
        List            m_recentsProjects;

};

#endif // RECENTPROJECTS_H
