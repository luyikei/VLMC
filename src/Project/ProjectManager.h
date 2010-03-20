/*****************************************************************************
 * ProjectManager.h: Manager the project loading and saving.
 *****************************************************************************
 * Copyright (C) 2008-2010 VideoLAN
 *
 * Authors: Hugo Beauzee-Luyssen <hugo@vlmc.org>
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

#ifndef PROJECTMANAGER_H
#define PROJECTMANAGER_H

#include "config.h"
#include <QObject>
#include <QDomDocument>
#include <QStringList>

#include "Singleton.hpp"

class   QFile;
class   QDomDocument;

#ifdef WITH_GUI
class   ProjectManager : public QObject
#else
class   ProjectManager : public QObject , public Singleton<ProjectManager>
#endif
{
    Q_OBJECT
    Q_DISABLE_COPY( ProjectManager );
public:
    static const QString    unNamedProject;
    static const QString    unSavedProject;

    void            loadProject( const QString& fileName );
    QStringList     recentsProjects() const;
    virtual bool    closeProject();
    virtual void    saveProject( bool saveAs = false );
    bool            loadEmergencyBackup();

    static void     signalHandler( int sig );

protected:
    /**
     *  This shouldn't be call directly.
     *  It's only purpose it to write the project for very specific cases.
     */
    void            __saveProject( const QString& fileName );
    void            emergencyBackup();
    static bool     isBackupFile( const QString& projectFile );
    void            appendToRecentProject( const QString& projectName );
    /**
     *  \brief      Get the project name
     *
     *  The project name will be either the project name given in the project wizard,
     *  or the filename without the extension. If the project is still unsaved,
     *  ProjectManager::unSavedProject is returned.
     *  \return     The project name.
     */
    QString         projectName() const;

protected:
    ProjectManager();
    ~ProjectManager();

protected:
    QFile*          m_projectFile;
    QStringList     m_recentsProjects;
    QString         m_projectName;
    QString         m_projectDescription;
    QDomDocument    *m_domDocument;
    bool            m_needSave;

    friend class    Singleton<ProjectManager>;

protected slots:
    void            loadTimeline();

signals:
    /**
     *  This signal is emitted when :
     *      - The project name has changed
     *      - The clean state has changed
     *      - The revision (if activated) has changed
     */
    void            projectUpdated( const QString& projectName, bool savedState );
    /**
     *  \brief      Used to signal that the project has been saved.
     *
     *  Right now, it is only used by the undo stack to flag the current state as clean.
     */
    void            projectSaved();
    void            projectClosed();
};

#endif // PROJECTMANAGER_H
