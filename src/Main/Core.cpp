/*****************************************************************************
 * Core.cpp: VLMC Base functions.
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

#include "Core.h"

#include <QCoreApplication>
#include <QDir>
#include <QtGlobal>
#include <QUndoStack>
#include <QStandardPaths>


#include <Backend/IBackend.h>
#include <EffectsEngine/EffectsEngine.h>
#include "Library/Library.h"
#include "Project/RecentProjects.h"
#include "Project/Workspace.h"
#include "Renderer/WorkflowRenderer.h"
#include <Settings/Settings.h>
#include <Tools/VlmcLogger.h>
#include "Workflow/MainWorkflow.h"

Core::Core()
{
    m_backend = Backend::getBackend();
    m_effectsEngine = new EffectsEngine;
    m_logger = new VlmcLogger;

    createSettings();
    m_currentProject = new Project( m_settings );
    m_library = new Library( m_currentProject->settings() );
    m_recentProjects = new RecentProjects( m_settings );
    m_workspace = new Workspace( m_settings );
    m_workflow = new MainWorkflow( m_currentProject->settings() );
    m_workflowRenderer = new WorkflowRenderer( Backend::getBackend(), m_workflow );
    m_undoStack = new QUndoStack;

    connect( m_undoStack, &QUndoStack::cleanChanged, m_currentProject, &Project::cleanChanged );
    connect( m_currentProject, &Project::projectSaved, m_undoStack, &QUndoStack::setClean );
    connect( m_library, &Library::cleanStateChanged, m_currentProject, &Project::libraryCleanChanged );
    connect( m_currentProject, SIGNAL( projectLoaded( QString, QString ) ),
             m_recentProjects, SLOT( projectLoaded( QString, QString ) ) );

    m_timer.start();
}

Core::~Core()
{
    m_settings->save();
    delete m_library;
    delete m_undoStack;
    delete m_workflowRenderer;
    delete m_workflow;
    delete m_currentProject;
    delete m_workspace;
    delete m_settings;
    delete m_logger;
    delete m_effectsEngine;
    delete m_backend;
}

void
Core::createSettings()
{
    QString configDir  = QStandardPaths::writableLocation( QStandardPaths::ConfigLocation );
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

bool
Core::loadProject(const QString& fileName)
{
    if ( fileName.isEmpty() == true )
        return false;
    //FIXME: What if the project was unsaved, and the user wants to cancel the operation?
    m_currentProject->load( fileName );

    return true;
}

bool
Core::newProject( const QString& projectName, const QString& projectPath )
{
    m_currentProject->newProject( projectName, projectPath );
    return true;
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

Project* Core::project()
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

QUndoStack*
Core::undoStack()
{
    return m_undoStack;
}

Library*
Core::library()
{
    return m_library;
}

qint64
Core::runtime()
{
    return m_timer.elapsed();
}

Core*
Core::instance()
{
    static Core core;
    return &core;
}
