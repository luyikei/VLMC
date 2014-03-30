/*****************************************************************************
 * ProjectManager.h: Manager the project loading and saving.
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

#ifndef PROJECTMANAGER_H
#define PROJECTMANAGER_H

#include "config.h"

class   QFile;
class   QDomDocument;
class   QXmlStreamWriter;

#include <QObject>
#include <QStringList>
#include <QDomElement>

class Settings;

class   IProjectManagerUiCb
{
public:
    enum SaveMode
    {
        Save,       // Save the project
        Discard,    // Discard it
        Cancel      // Don't do anything
    };

    virtual ~IProjectManagerUiCb() {}

    /**
     * @brief shouldSaveBeforeClose Ask the user if she wants to save the project in case
     *                              it's about to be closed
     * @return True if the project should be saved. False if changes are to be discarded.
     */
    virtual SaveMode    shouldSaveBeforeClose() = 0;

    /**
     * @brief getProjectFile    Ask the user where to save a new project
     * @param defaultPath       A default project location, if any.
     * @param isOpen            True if the project will be opened, false if it's about to
     *                          be saved.
     * @return The selected project file
     */
    virtual QString    getProjectFile( const QString& defaultPath, bool isOpen ) = 0;

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

class   ProjectManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY( ProjectManager )

public:
    static const QString            unNamedProject;
    static const QString            backupSuffix;



    ProjectManager( Settings *projectSettings , Settings *vlmcSettings );
    ~ProjectManager();

    void            setProjectManagerUi( IProjectManagerUiCb* projectManagerUi );
    void            removeProject( const QString& projectPath );
    bool            closeProject();

    void            save();
    void            saveAs();
    bool            loadEmergencyBackup();
    void            emergencyBackup();
    bool            hasProjectLoaded() const;
    void            newProject( const QString& projectName, const QString &workspacePath );

    void            loadProject();
    /**
     *  \brief      Check for a project backup file, and load the appropriate file,
     *              according to the user input.
     *
     *  if an outdated project backup is found, the used is asked if she wants to delete
     *  it.
     */
    void            loadProject( const QString& fileName );
private:
    void            saveProject( const QString& filename );
    /**
     *  \brief      Save the timline.
     *
     */
    void            saveTimeline(QXmlStreamWriter& project);
    static bool     isBackupFile( const QString& projectFile );
    /**
     *  \brief      Get the project name
     *
     *  The project name will be either the project name given in the project wizard,
     *  or the filename without the extension. If the project is still unsaved,
     *  ProjectManager::unSavedProject is returned.
     *  \return     The project name.
     */
    QString         projectName() const;

    void            failedToLoad( const QString& reason ) const;
    void            loadTimeline( const QDomElement& ){}
    QString         createAutoSaveOutputFileName( const QString& baseName ) const;


protected:
    QFile*                  m_projectFile;
    // We list recent projects as a list of [ProjectName,ProjectPath].
    // Since this is handled as a QVariant, arrays don't work.
    QString                 m_projectName;
    QString                 m_projectDescription;
    QDomDocument*           m_domDocument;
    bool                    m_isClean;
    bool                    m_libraryCleanState;
    IProjectManagerUiCb*    m_projectManagerUi;
    Settings*               m_projectSettings;
    Settings*               m_vlmcSettings;

public slots:
    void            cleanChanged( bool val );
    void            libraryCleanChanged( bool val );


private slots:
    void            loadWorkflow();
    void            projectNameChanged( const QVariant& projectName );
    void            autoSaveRequired();

signals:
    /**
     *  This signal is emitted when :
     *      - The project name has changed
     *      - The clean state has changed
     *      - The revision (if activated) has changed
     */
    void            projectUpdated( const QString& projectName );

    /**
     *  \brief      Used to signal that the project has been saved.
     *
     *  Right now, it is only used by the undo stack to flag the current state as clean.
     */
    void            projectSaved();

    /**
      * @brief cleanStateChanged    Emited when the clean state changes.
      *
      * This state is a combination of both library & undoStack clean states.
      */
    void            cleanStateChanged( bool value );

    /**
     * @brief projectLoaded Emited when a project is loaded (which also include a project
     *                      being created)
     * @param projectName   The project name
     * @param projectPath   The path to the project file
     */
    void            projectLoaded( const QString& projectName, const QString& projectPath );
};

#endif // PROJECTMANAGER_H
