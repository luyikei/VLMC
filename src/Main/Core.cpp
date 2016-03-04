/*****************************************************************************
 * Core.cpp: VLMC Base functions.
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

#include "Core.h"

#include <QCoreApplication>
#include <QDir>
#include <QtGlobal>
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
# include <QStandardPaths>
#else
# include <QDesktopServices>
#endif

#include <Backend/IBackend.h>
#include <EffectsEngine/EffectsEngine.h>
#include <Settings/Settings.h>
#include <Tools/VlmcLogger.h>
#include "Project/AutomaticBackup.h"
#include "Project/RecentProjects.h"
#include "Project/Workspace.h"
#include "Renderer/WorkflowRenderer.h"
#include "Workflow/MainWorkflow.h"

Core::Core()
    : m_currentProject( NULL )
{
    m_backend = Backend::getBackend();
    m_effectsEngine = new EffectsEngine;
    m_logger = new VlmcLogger;

    createSettings();
    m_recentProjects = new RecentProjects( m_settings );
    m_automaticBackup = new AutomaticBackup( m_settings );
    m_workspace = new Workspace( m_settings );
    m_workflow = new MainWorkflow;
    m_workflowRenderer = new WorkflowRenderer( Backend::getBackend(), m_workflow );
}

Core::~Core()
{
    m_settings->save();
    delete m_workflowRenderer;
    delete m_workflow;
    delete m_currentProject;
    delete m_workspace;
    delete m_automaticBackup;
    delete m_settings;
    delete m_logger;
    delete m_effectsEngine;
    delete m_backend;
}

void
Core::createSettings()
{
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    QString configDir = QStandardPaths::writableLocation( QStandardPaths::ConfigLocation );
#else
    QString configDir = QDesktopServices::storageLocation( QDesktopServices::DataLocation );
#endif
    QString configPath = configDir + QDir::separator() + qApp->organizationName()
            + QDir::separator() + qApp->applicationName() + ".conf";
    m_settings = new Settings( configPath );
    m_settings->createVar( SettingValue::String, "vlmc/WorkspaceLocation", "",
                                    QT_TRANSLATE_NOOP( "Settings", "Workspace location" ),
                                    QT_TRANSLATE_NOOP( "Settings", "VLMC's workspace location" ),
                                    SettingValue::Nothing );
    m_settings->createVar( SettingValue::Bool, "private/FirstLaunchDone", false, "", "", SettingValue::Private );
}

Backend::IBackend*
Core::backend()
{
    return m_backend;
}

EffectsEngine*
Core::effectsEngine()
{
    return m_effectsEngine;
}

VlmcLogger*
Core::logger()
{
    return m_logger;
}

RecentProjects*
Core::recentProjects()
{
    return m_recentProjects;
}

AutomaticBackup*
Core::automaticBackup()
{
    return m_automaticBackup;
}

bool
Core::loadProject(const QString& fileName)
{
    if ( fileName.isEmpty() == true )
        return false;
    QFile* projectFile = new QFile( fileName );
    if ( projectFile->exists() == false )
        return false;
    //FIXME: What if the project was unsaved, and the user wants to cancel the operation?
    delete m_currentProject;

    //FIXME: Doesn't check for proper project loading
    m_currentProject = new Project( projectFile );
    m_automaticBackup->setProject( m_currentProject );
    m_recentProjects->setProject( m_currentProject );
    emit projectLoading( m_currentProject );
    return true;
}

bool Core::newProject(const QString& projectName, const QString& projectPath)
{
    delete m_currentProject;
    //FIXME: Doesn't check for proper project creation
    m_currentProject = new Project( projectName, projectPath );
    return true;
}

bool
Core::restoreProject()
{
    //FIXME: This doesn't make sense when no GUI is attached, so I'm not sure it fits in the Core class.
    Q_ASSERT( m_currentProject == NULL );
    QFile* backupFile = Project::emergencyBackupFile();
    if ( backupFile == NULL )
        return false;
    //FIXME: This lacks error handling
    m_currentProject = new Project( backupFile );
    return true;
}

bool Core::isProjectLoaded()
{
    return m_currentProject != NULL;
}

Settings*
Core::settings()
{
    return m_settings;
}


Workspace*
Core::workspace()
{
    return m_workspace;
}

Project* Core::currentProject()
{
    return m_currentProject;
}

WorkflowRenderer*
Core::workflowRenderer()
{
    return m_workflowRenderer;
}

MainWorkflow*
Core::workflow()
{
    return m_workflow;
}

Core*
Core::getInstance()
{
    static Core core;
    return &core;
}
