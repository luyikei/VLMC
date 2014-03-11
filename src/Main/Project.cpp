/*****************************************************************************
 * Project.cpp: Handles all core project components
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

#include "Project.h"

#include <QUndoStack>

#include "Library/Library.h"
#include "Workflow/MainWorkflow.h"
#include "Project/Workspace.h"
#include "Settings/Settings.h"

Project::Project()
{
    m_library = new Library;
    m_settings = new Settings( QString() );
    m_undoStack = new QUndoStack;
    m_workflow = new MainWorkflow;
    m_workspace = new Workspace( m_settings );
}

Project::~Project()
{
    delete m_workspace;
    delete m_workflow;
    delete m_undoStack;
    delete m_settings;
    delete m_library;
}

Library*
Project::library()
{
    return m_library;
}

MainWorkflow*
Project::workflow()
{
    return m_workflow;
}

QUndoStack*
Project::undoStack()
{
    return m_undoStack;
}

Settings*
Project::settings()
{
    return m_settings;
}

Workspace*
Project::workspace()
{
    return m_workspace;
}
