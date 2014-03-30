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

#include "config.h"

#include <QObject>

#include "Tools/Singleton.hpp"

class QDomDocument;
class QFile;
class QString;
class QUndoStack;
class QXmlStreamWriter;

class Library;
class MainWorkflow;
class ProjectManager;
class Settings;
class Workspace;

class   IProjectUiCb
{
public:
    enum SaveMode
    {
        Save,       // Save the project
        Discard,    // Discard it
        Cancel      // Don't do anything
    };

    virtual ~IProjectUiCb() {}

    /**
     * @brief shouldSaveBeforeClose Ask the user if she wants to save the project in case
     *                              it's about to be closed
     * @return True if the project should be saved. False if changes are to be discarded.
     */
    virtual SaveMode    shouldSaveBeforeClose() = 0;

    /**
     * @brief getProjectFile    Ask the user where to save a new project
     * @param defaultPath       A default project location, if any.
     * @return The selected project file
     */
    virtual QString    getProjectFileDestination( const QString& defaultPath ) = 0;

    /**
     * @brief shouldLoadBackupFile
     * @return True if the user wants to load the backup file.
     */
    virtual bool    shouldLoadBackupFile() = 0;

    /**
     * @brief shouldDeleteOutdatedBackupFile
     * @return True if the user wants to delete this backup file
     */
    virtual bool    shouldDeleteOutdatedBackupFile() = 0;
};

class Project : public QObject, public Singleton<Project>
{
    Q_OBJECT
    public:
        static const QString            unNamedProject;
        static const QString            backupSuffix;

    public:
        // Main entry point for loading a project
        static bool     load( const QString& fileName );
        static bool     create(const QString& projectName, const QString& projectPath );
        static bool     isProjectLoaded();
        void            save();
        void            saveAs();
        bool            loadEmergencyBackup();
        void            emergencyBackup();

    private:
        Q_DISABLE_COPY( Project );
        Project();
        ~Project();

        /**
         *  @brief      Check for a project backup file, and load the appropriate file,
         *              according to the user input.
         *  @param fileName     The path of the project file to load. This is expected to be
         *                      an absolute file path.
         *  if an outdated project backup is found, the used is asked if she wants to delete
         *  it.
         */
        bool                loadProject( const QString& fileName );
        /**
         * @brief connectComponents     Connects project specific components' signals & slots
         */
        void                connectComponents();
        /**
         * @brief checkBackupFile   Check for potential backup files and handle them
         * @param projectFile       The project file being opened
         * @return                  A path to a backup project if any. projectFile otherwise.
         */
        QString             checkBackupFile( const QString& projectFile );
        void                initSettings();
        QString             name();
        bool                closeProject();
        void                saveProject( const QString& filename );
        void                newProject( const QString& projectName, const QString &workspacePath );


    private slots:
        void                cleanChanged( bool val );
        void                libraryCleanChanged( bool val );
        void                projectNameChanged( const QVariant& projectName );
        void                loadWorkflow();
        void                autoSaveRequired();

    signals:
        /**
         *  This signal is emitted when :
         *      - The project name has changed
         *      - The clean state has changed
         *      - The revision (if activated) has changed
         */
        void                projectUpdated( const QString& projectName );

        /**
         *  \brief      Used to signal that the project has been saved.
         *
         *  Right now, it is only used by the undo stack to flag the current state as clean.
         */
        void                projectSaved();

        /**
          * @brief cleanStateChanged    Emited when the clean state changes.
          *
          * This state is a combination of both library & undoStack clean states.
          */
        void                cleanStateChanged( bool value );

        /**
         * @brief projectLoaded Emited when a project is loaded (which also include a project
         *                      being created)
         * @param projectName   The project name
         * @param projectPath   The path to the project file
         */
        void                projectLoaded( const QString& projectName, const QString& projectPath );

    private:
        QFile*              m_projectFile;
        QString             m_projectName;
        QDomDocument*       m_domDocument;
        bool                m_isClean;
        bool                m_libraryCleanState;
        IProjectUiCb*       m_projectManagerUi;

    ///////////////////////////////////
    // Dependent components part below:
    public:
        Library*            library();
        MainWorkflow*       workflow();
        QUndoStack*         undoStack();
        Settings*           settings();
        Workspace*          workspace();

    private:
        Library*            m_library;
        MainWorkflow*       m_workflow;
        QUndoStack*         m_undoStack;
        Settings*           m_settings;
        Workspace*          m_workspace;

    friend class Singleton<Project>;
};

#endif // PROJECT_H
