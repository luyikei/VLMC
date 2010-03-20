/*****************************************************************************
 * GUIProjectManager.h: Handle the GUI part of the project managing
 *****************************************************************************
 * Copyright (C) 2008-2010 VideoLAN
 *
 * Authors: Hugo Beauzée-Luyssen <beauze.h@gmail.com>
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

#ifndef GUIPROJECTMANAGER_H
#define GUIPROJECTMANAGER_H

#include "ProjectManager.h"

class   QTimer;

class GUIProjectManager : public ProjectManager, public Singleton<GUIProjectManager>
{
    Q_OBJECT

public:
    GUIProjectManager();

    bool            askForSaveIfModified();
    void            newProject( const QString& projectName );
    /**
     *  \brief      Ask the user for the project file she wants to load.
     *
     *  \return     The project to load.
     */
    QString         acquireProjectFileName();
    void            saveProject( bool saveAs = false );
    /**
     *  \brief      Ask the project manager to close current project.
     *
     *  This can fail, as the user will be asked if he wants to save the current project.
     *  If she selects discard, the project closing procedure is aborted.
     *  \return     true if the project has been closed. false otherwise.
     */
    bool            closeProject();
    bool            needSave() const;

private:
    bool            createNewProjectFile( bool saveAs );

private:
    QTimer*         m_timer;

private slots:
    void            projectNameChanged( const QVariant& projectName );
    void            autoSaveRequired();
    void            cleanChanged( bool val );
    void            automaticSaveEnabledChanged( const QVariant& enabled );
    void            automaticSaveIntervalChanged( const QVariant& interval );

    friend class    Singleton<GUIProjectManager>;
};

#endif // GUIPROJECTMANAGER_H
