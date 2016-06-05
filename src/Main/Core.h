/*****************************************************************************
 * Core.h: VLMC Base functions.
 *****************************************************************************
 * Copyright (C) 2008-2016 the VLMC team
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

#ifndef CORE_H
#define CORE_H

class AutomaticBackup;
class Library;
class MainWorkflow;
class MediaLibrary;
class Project;
class RecentProjects;
class Settings;
class VlmcLogger;
class Workspace;

namespace Backend
{
    class IBackend;
}

#include <QElapsedTimer>
#include "Tools/Singleton.hpp"

class Core : public ScopedSingleton<Core>
{
    public:
        Backend::IBackend*      backend();
        Settings*               settings();
        VlmcLogger*             logger();
        RecentProjects*         recentProjects();
        Workspace*              workspace();
        Project*                project();
        MainWorkflow*           workflow();
        Library*                library();
        MediaLibrary*           mediaLibrary();
        /**
         * @brief runtime returns the application runtime
         */
        qint64                  runtime();

        bool                    loadProject( const QString& fileName );
        bool                    newProject( const QString& projectName, const QString& projectPath );

    private:
        Core();
        ~Core();

        void                    createSettings();
        void                    connectComponents();

    private:
        Backend::IBackend*      m_backend;
        Settings*               m_settings;
        VlmcLogger*             m_logger;
        RecentProjects*         m_recentProjects;
        Workspace*              m_workspace;
        Project*                m_currentProject;
        MainWorkflow*           m_workflow;
        Library*                m_library;
        std::unique_ptr<MediaLibrary> m_ml;
        QElapsedTimer           m_timer;

        friend Singleton_t::AllowInstantiation;
};

#endif // CORE_H
