/*****************************************************************************
 * Project.h: Handles all core project components
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

#ifndef PROJECT_H
#define PROJECT_H

#include "Tools/Singleton.hpp"

class QUndoStack;

class Library;
class MainWorkflow;
class ProjectManager;
class Settings;
class Workspace;

class Project : public Singleton<Project>
{
private:
    Project();
    ~Project();

public:
    Library*            library();
    MainWorkflow*       workflow();
    QUndoStack*         undoStack();
    Settings*           settings();
    Workspace*          workspace();
    ProjectManager*     projectManager();

private:
    Library*            m_library;
    MainWorkflow*       m_workflow;
    QUndoStack*         m_undoStack;
    Settings*           m_settings;
    Workspace*          m_workspace;
    ProjectManager*     m_projectManager;

    friend class Singleton<Project>;
};

#endif // PROJECT_H
