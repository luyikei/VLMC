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

Core::Core()
{
    m_backend = Backend::getBackend();
    m_effectsEngine = new EffectsEngine;
    m_logger = new VlmcLogger;

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    QString configDir = QStandardPaths::writableLocation( QStandardPaths::ConfigLocation );
#else
    QString configDir = QDesktopServices::storageLocation( QDesktopServices::DataLocation );
#endif
    QString configPath = configDir + QDir::separator() + qApp->organizationName()
            + QDir::separator() + qApp->applicationName() + ".conf";
    m_settings = new Settings( configPath );
    m_recentProjects = new RecentProjects( m_settings );
    m_automaticBackup = new AutomaticBackup( m_settings );
    m_workspace = new Workspace( m_settings );
}

Core::~Core()
{
    delete m_workspace;
    delete m_automaticBackup;
    delete m_settings;
    delete m_logger;
    delete m_effectsEngine;
    delete m_backend;
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

void
Core::onProjectLoaded( Project* project )
{
    m_automaticBackup->setProject( project );
    m_recentProjects->setProject( project );

    emit projectLoading( project );
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
