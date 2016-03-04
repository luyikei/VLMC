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

#include "ILoadSave.h"

class QDomDocument;
class QFile;
class QString;
class QUndoStack;
class QXmlStreamWriter;

class AutomaticBackup;
class IProjectUiCb;
class Library;
class MainWorkflow;
class ProjectManager;
class Settings;

class Project : public QObject
{
    Q_OBJECT
    public:
        static const QString            unNamedProject;
        static const QString            backupSuffix;

    public:
        Q_DISABLE_COPY( Project );
        Project( QFile* projectFile );
        Project( const QString& projectName, const QString& projectPath );

        virtual ~Project();

        void            save();
        void            saveAs(const QString& fileName);
        void            emergencyBackup();
        bool            registerLoadSave( ILoadSave* loadSave );
        bool            isClean() const;

    public:
        static QFile* emergencyBackupFile();

    private:
        /**
         *  @brief      Check for a project backup file, and load the appropriate file,
         *              according to the user input.
         *  @param fileName     The path of the project file to load. This is expected to be
         *                      an absolute file path.
         *  if an outdated project backup is found, the used is asked if she wants to delete
         *  it.
         */
        bool                load();
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
        void                saveProject( const QString& filename );
        void                newProject( const QString& projectName, const QString& projectPath );


    private slots:
        void                cleanChanged( bool val );
        void                libraryCleanChanged( bool val );
        void                projectNameChanged( const QVariant& projectName );
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
        bool                m_isClean;
        bool                m_libraryCleanState;
        IProjectUiCb*       m_projectManagerUi;
        QList<ILoadSave*>   m_loadSave;

    ///////////////////////////////////
    // Dependent components part below:
    public:
        Library*            library();
        QUndoStack*         undoStack();
        Settings*           settings();

    private:
        Library*            m_library;
        QUndoStack*         m_undoStack;
        Settings*           m_settings;
};

#endif // PROJECT_H
