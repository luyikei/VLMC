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
class EffectsEngine;
class Library;
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
#include <QElapsedTimer>

class Core : public QObject
{
    Q_OBJECT

    public:
        Backend::IBackend*      backend();
        EffectsEngine*          effectsEngine();
        Settings*               settings();
        VlmcLogger*             logger();
        RecentProjects*         recentProjects();
        Workspace*              workspace();
        Project*                project();
        WorkflowRenderer*       workflowRenderer();
        MainWorkflow*           workflow();
        QUndoStack*             undoStack();
        Library*                library();
        /**
         * @brief runtime returns the application runtime
         */
        qint64                  runtime();

        bool                    loadProject( const QString& fileName );
        bool                    newProject( const QString& projectName, const QString& projectPath );

        static Core*            getInstance();

    private:
        Core();
        virtual ~Core();

        void                    createSettings();
        void                    connectComponents();

    private:
        Backend::IBackend*      m_backend;
        EffectsEngine*          m_effectsEngine;
        Settings*               m_settings;
        VlmcLogger*             m_logger;
        RecentProjects*         m_recentProjects;
        Workspace*              m_workspace;
        Project*                m_currentProject;
        MainWorkflow*           m_workflow;
        WorkflowRenderer*       m_workflowRenderer;
        QUndoStack*             m_undoStack;
        Library*                m_library;
        QElapsedTimer           m_timer;
};

#endif // CORE_H
