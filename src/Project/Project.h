/*****************************************************************************
 * Project.h: Handles all core project components
 *****************************************************************************
 * Copyright (C) 2008-2016 VideoLAN
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

class QFile;
class QString;
class QTimer;

class AutomaticBackup;
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
        Q_DISABLE_COPY( Project )
        Project( Settings* settings );

        virtual ~Project();

        void            save();
        void            saveAs( const QString& fileName );
        void            newProject( const QString& projectName, const QString& projectFilePath );
        /**
         *  @brief          Check for a project backup file, and load the appropriate file,
         *                  according to the user input.
         *  @param fileName The path of the project file to load. This is expected to be
         *                  an absolute file path.
         */
        bool            load( const QString& path );
        void            emergencyBackup();
        bool            isClean() const;
        void            closeProject();
        bool            hasProjectFile() const;
        /**
         * @brief removeBackupFile Removes the current project backup file, if any
         */
        void            removeBackupFile();

        // Settings getters
        QString         name() const;
        double          fps() const;
        unsigned int    width() const;
        unsigned int    height() const;
        QString         aspectRatio() const;
        unsigned int    audioBitrate() const;
        unsigned int    videoBitrate() const;
        unsigned int    sampleRate() const;
        unsigned int    nbChannels() const;

    public:
        static QFile* emergencyBackupFile();

    private:
        void                initSettings();
        void                saveProject( const QString& filename );


    public slots:
        void                cleanChanged( bool val );
        void                libraryCleanChanged( bool val );
        void                autoSaveRequired();
        void                autoSaveEnabledChanged( const QVariant& enabled );
        void                autoSaveIntervalChanged( const QVariant& interval );


    signals:
        /**
         *  This signal is emitted when :
         *      - The project name has changed
         *      - The revision (if activated) has changed
         */
        void                projectNameChanged( const QString& projectName );

        /**
         *  \brief      Used to signal that the project has been saved.
         */
        void                projectSaved( const QString& projectName, const QString& projectFilePath );

        /**
          * @brief cleanStateChanged    Emited when the clean state changes.
          *
          * This state is a combination of both library & undoStack clean states.
          */
        void                cleanStateChanged( bool value );

        void                projectLoading( const QString& projectName );
        void                projectLoaded( const QString& projectName, const QString& projectFilePath );
        void                projectClosed();
        void                backupProjectLoaded();
        void                outdatedBackupFileFound();
        void                fpsChanged( double fps );

    private:
        QFile*              m_projectFile;
        bool                m_isClean;
        bool                m_libraryCleanState;
        QTimer*             m_timer;

    ///////////////////////////////////
    // Dependent components part below:
    public:
        Settings*           settings();

    private:
        Settings*           m_settings;
};

#endif // PROJECT_H
