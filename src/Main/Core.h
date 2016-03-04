/*****************************************************************************
 * Core.h: VLMC Base functions.
 *****************************************************************************
 * Copyright (C) 2008-2014 the VLMC team
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
class EffectsEngine;
class MainWorkflow;
class NotificationZone;
class Project;
class RecentProjects;
class Settings;
class VlmcLogger;
class Workspace;
class WorkflowRenderer;

class QUndoStack;

namespace Backend
{
    class IBackend;
}

#include <QObject>

class Core : public QObject
{
    Q_OBJECT

    public:
        Backend::IBackend*      backend();
        EffectsEngine*          effectsEngine();
        Settings*               settings();
        VlmcLogger*             logger();
        RecentProjects*         recentProjects();
        AutomaticBackup*        automaticBackup();
        Workspace*              workspace();
        Project*                currentProject();
        WorkflowRenderer*       workflowRenderer();
        MainWorkflow*           workflow();
        QUndoStack*             undoStack();


        bool                    loadProject( const QString& fileName );
        bool                    newProject( const QString& projectName, const QString& projectPath );
        bool                    restoreProject();
        bool                    isProjectLoaded();

        static Core*            getInstance();

    signals:
        /**
         * @brief projectLoaded Will be emited right before a project starts loading.
         *
         * When this is emited, all Project related instances are created. The project itself
         * however won't be loaded (no settings, no media, ...)
         * This should be used with Qt::DirectConnection in order not to miss any media/clip
         * loading events.
         */
        void                    projectLoading( Project* project );

    private:
        Core();
        virtual ~Core();

        void                    createSettings();

    private:
        Backend::IBackend*      m_backend;
        EffectsEngine*          m_effectsEngine;
        Settings*               m_settings;
        VlmcLogger*             m_logger;
        RecentProjects*         m_recentProjects;
        AutomaticBackup*        m_automaticBackup;
        Workspace*              m_workspace;
        Project*                m_currentProject;
        MainWorkflow*           m_workflow;
        WorkflowRenderer*       m_workflowRenderer;
        QUndoStack*             m_undoStack;
};

#endif // CORE_H
