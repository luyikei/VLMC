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

#include <QDir>
#include <QSettings>
#include <QtDebug>
#include <QXmlStreamWriter>

#include <errno.h>
#include <signal.h>

const QString   ProjectManager::unNamedProject = tr( "<Unnamed project>" );
const QString   ProjectManager::unSavedProject = tr( "<Unsaved project>" );

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
    VLMC_CREATE_PROJECT_STRING( "general/VLMCWorkspace", QDir::homePath(),
                                QT_TRANSLATE_NOOP( "PreferenceWidget", "Workspace location" ),
                                QT_TRANSLATE_NOOP( "PreferenceWidget", "The place where all project's videos will be stored" ) );

    VLMC_CREATE_PROJECT_STRING( "general/ProjectName", unNamedProject,
                                QT_TRANSLATE_NOOP( "PreferenceWidget", "Project name" ),
                                QT_TRANSLATE_NOOP( "PreferenceWidget", "The project name" ) );

    //We have to wait for the library to be loaded before loading the workflow
    connect( Library::getInstance(), SIGNAL( projectLoaded() ), this, SLOT( loadWorkflow() ) );
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

    MainWorkflow::getInstance()->loadProject( root );
    loadTimeline( root );
    SettingsManager::getInstance()->load( root );
    emit projectUpdated( projectName(), true );
    emit projectLoaded();
    if ( m_projectFile != NULL )
        appendToRecentProject( m_projectFile->fileName() );
    delete m_domDocument;
}

void    ProjectManager::loadProject( const QString& fileName )
{
    //Don't print an error. The user most likely canceled the open project dialog.
    if ( fileName.isEmpty() == true )
        return ;

    if ( closeProject() == false )
        return ;
    m_projectFile = new QFile( fileName );
    if ( m_projectFile->open( QFile::ReadOnly ) == false )
    {
        failedToLoad( tr( "Can't open project file. (%1)" ).arg( m_projectFile->errorString() ) );
        return ;
    }
    m_projectFile->close();

    m_domDocument = new QDomDocument;
    m_domDocument->setContent( m_projectFile );
#ifdef WITH_GUI
    m_needSave = false;
#endif
    if ( ProjectManager::isBackupFile( fileName ) == true )
    {
        //Delete the project file representation, so the next time the user
        //saves its project, vlmc will ask him where to save it.
        delete m_projectFile;
        m_projectFile = NULL;
    }

    QDomElement     root = m_domDocument->documentElement();

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
    {
        name = m_projectFile->fileName();
        name += "backup";
        __saveProject( name );
    }
    else
    {
       name = QDir::currentPath() + "/unsavedproject.vlmcbackup";
        __saveProject( name );
    }
    QSettings   s;
    s.setValue( "EmergencyBackup", name );
    s.sync();
}

bool    ProjectManager::isBackupFile( const QString& projectFile )
{
    return projectFile.endsWith( "backup" );
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
    m_projectName = QString();
    //This one is for the mainwindow, to update the title bar
    emit projectUpdated( projectName(), true );
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
