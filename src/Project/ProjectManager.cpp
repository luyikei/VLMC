/*****************************************************************************
 * ProjectManager.cpp: Manager the project loading and saving.
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

#include "config.h"

#include "Library.h"
#include "MainWorkflow.h"
#include "project/GuiProjectManager.h"
#include "ProjectManager.h"
#include "SettingsManager.h"
#include "Workspace.h"

#include <QDir>
#include <QSettings>
#include <QtDebug>
#include <QXmlStreamWriter>

#include <errno.h>
#include <signal.h>

const QString   ProjectManager::unNamedProject = tr( "<Unnamed project>" );
const QString   ProjectManager::unSavedProject = tr( "<Unsaved project>" );
const QString   ProjectManager::backupSuffix = "~";

ProjectManager::ProjectManager() : m_projectFile( NULL ), m_needSave( false )
{
    QSettings s;
    m_recentsProjects = s.value( "RecentsProjects" ).toStringList();

    VLMC_CREATE_PROJECT_DOUBLE( "video/VLMCOutputFPS", 29.97,
                                QT_TRANSLATE_NOOP( "PreferenceWidget", "Output video FPS" ),
                                QT_TRANSLATE_NOOP( "PreferenceWidget", "Frame Per Second used when previewing and rendering the project" ) );
    VLMC_CREATE_PROJECT_INT( "video/VideoProjectWidth", 480,
                             QT_TRANSLATE_NOOP( "PreferenceWidget", "Video width" ),
                             QT_TRANSLATE_NOOP( "PreferenceWidget", "Width resolution of the output video" ) );
    VLMC_CREATE_PROJECT_INT( "video/VideoProjectHeight", 300,
                             QT_TRANSLATE_NOOP( "PreferenceWidget", "Video height" ),
                             QT_TRANSLATE_NOOP( "PreferenceWidget", "Height resolution of the output video" ) );
    VLMC_CREATE_PROJECT_INT( "audio/AudioSampleRate", 0,
                             QT_TRANSLATE_NOOP( "PreferenceWidget", "Audio samplerate" ),
                             QT_TRANSLATE_NOOP( "PreferenceWidget", "Output project audio samplerate" ) );
    VLMC_CREATE_PREFERENCE_STRING( "general/VLMCWorkspace", QDir::homePath(),
                                QT_TRANSLATE_NOOP( "PreferenceWidget", "Workspace location" ),
                                QT_TRANSLATE_NOOP( "PreferenceWidget", "The place where all project's medias will be stored" ) );

    VLMC_CREATE_PROJECT_STRING( "general/ProjectName", unNamedProject,
                                QT_TRANSLATE_NOOP( "PreferenceWidget", "Project name" ),
                                QT_TRANSLATE_NOOP( "PreferenceWidget", "The project name" ) );

    VLMC_CREATE_PRIVATE_PROJECT_STRING( "general/ProjectDir", "" );

    //We have to wait for the library to be loaded before loading the workflow
    connect( Library::getInstance(), SIGNAL( projectLoaded() ), this, SLOT( loadWorkflow() ) );
    //Create the workspace:
    Workspace::getInstance();
}

ProjectManager::~ProjectManager()
{
    // Write uncommited change to the disk
    QSettings s;
    s.sync();

    if ( m_projectFile != NULL )
        delete m_projectFile;
}


QStringList ProjectManager::recentsProjects() const
{
    return m_recentsProjects;
}

void    ProjectManager::loadWorkflow()
{
    QDomElement     root = m_domDocument->documentElement();
    bool            savedState;

    MainWorkflow::getInstance()->loadProject( root );
    loadTimeline( root );
    if ( m_projectFile != NULL )
    {
        appendToRecentProject( m_projectFile->fileName() );
        savedState = true;
    }
    else
        savedState = false;
    emit projectUpdated( projectName(), savedState );

    delete m_domDocument;
}

void    ProjectManager::loadProject( const QString& fileName )
{
    //FIXME:this is probably useless, as this is handled by the gui part now.
    //Don't print an error. The user most likely canceled the open project dialog.
    if ( fileName.isEmpty() == true )
        return ;

    if ( closeProject() == false )
        return ;
    m_projectFile = new QFile( fileName );
    if ( m_projectFile->open( QFile::ReadOnly ) == false )
    {
        failedToLoad( tr( "Can't open project file. (%1)" ).arg( m_projectFile->errorString() ) );
        delete m_projectFile;
        m_projectFile = NULL;
        return ;
    }
    m_projectFile->close();

    m_domDocument = new QDomDocument;
    m_domDocument->setContent( m_projectFile );
    if ( ProjectManager::isBackupFile( fileName ) == true )
    {
        //Delete the project file representation, so the next time the user
        //saves its project, vlmc will ask him where to save it.
        delete m_projectFile;
        m_projectFile = NULL;
        m_needSave = true;
    }

    QDomElement     root = m_domDocument->documentElement();

    //Load settings first, as it contains some informations about the workspace.
    SettingsManager::getInstance()->load( root );
    QString     workspacePath = VLMC_GET_STRING("general/VLMCWorkspace");
    QString     projectName = VLMC_PROJECT_GET_STRING("general/ProjectName");
    QString     projectPath = workspacePath + '/' + projectName.replace( " ", "_" );
    SettingsManager::getInstance()->setValue( "general/ProjectDir", projectPath, SettingsManager::Project );
    Library::getInstance()->loadProject( root );
}

void    ProjectManager::__saveProject( const QString &fileName )
{
    QByteArray          projectString;

    QXmlStreamWriter    project( &projectString );

    project.setAutoFormatting( true );
    project.writeStartDocument();
    project.writeStartElement( "vlmc" );

    Library::getInstance()->saveProject( project );
    MainWorkflow::getInstance()->saveProject( project );
    SettingsManager::getInstance()->save( project );
    saveTimeline( project );

    project.writeEndElement();
    project.writeEndDocument();

    QFile               file( fileName );
    file.open( QFile::WriteOnly );
    file.write( projectString );
}

void    ProjectManager::emergencyBackup()
{
    QString     name;

    if ( m_projectFile != NULL )
        name = createAutoSaveOutputFileName( m_projectFile->fileName() );
    else
       name = createAutoSaveOutputFileName( QDir::currentPath() + "/unsavedproject" );
    __saveProject( name );
    QSettings   s;
    s.setValue( "EmergencyBackup", name );
    s.sync();
}

bool    ProjectManager::isBackupFile( const QString& projectFile )
{
    return projectFile.endsWith( ProjectManager::backupSuffix );
}

QString
ProjectManager::createAutoSaveOutputFileName( const QString& baseName ) const
{
    return baseName + ProjectManager::backupSuffix;
}


void    ProjectManager::appendToRecentProject( const QString& projectFile )
{
    // Append the item to the recents list
    m_recentsProjects.removeAll( projectFile );
    m_recentsProjects.prepend( projectFile );
    while ( m_recentsProjects.count() > 15 )
        m_recentsProjects.removeLast();

    QSettings s;
    s.setValue( "RecentsProjects", m_recentsProjects );
}

QString ProjectManager::projectName() const
{
    if ( m_projectName.isEmpty() == true )
    {
        if ( m_projectFile != NULL )
        {
            QFileInfo       fInfo( *m_projectFile );
            return fInfo.baseName();
        }
        return ProjectManager::unSavedProject;
    }
    return m_projectName;
}

bool
ProjectManager::closeProject()
{
    if ( m_projectFile != NULL )
    {
        delete m_projectFile;
        m_projectFile = NULL;
    }
    m_needSave = false;
    m_projectName = QString();
    //This one is for every part that need to clean something when the project is closed.
    emit projectClosed();
    return true;
}

void
ProjectManager::saveProject( const QString &outputFileName )
{
    __saveProject( outputFileName );
    emit projectSaved();
    emit projectUpdated( projectName(), true );
}

bool
ProjectManager::loadEmergencyBackup()
{
    QSettings   s;
    QString lastProject = s.value( "EmergencyBackup" ).toString();
    if ( QFile::exists( lastProject ) == true )
    {
        loadProject(  lastProject );
        m_needSave = true;
        return true;
    }
    return false;
}

void
ProjectManager::failedToLoad( const QString &reason ) const
{
    //When running in server mode, we can't do anything without a project file.
    qCritical() << tr( "Failed to load the project file: %1. Aborting." ).arg( reason );
    abort();
}

QString
ProjectManager::outputFileName() const
{
    return m_projectFile->fileName();
}
