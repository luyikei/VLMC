/*****************************************************************************
 * ProjectManager.cpp: Manager the project loading and saving.
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

#include "config.h"

#include "Library.h"
#include "MainWorkflow.h"
#include "project/GuiProjectManager.h"
#include "ProjectManager.h"
#include "SettingsManager.h"
#include "Timeline.h"
#include "VlmcDebug.h"
#include "WorkflowRenderer.h"
#include "Workspace.h"

#include <QDir>
#include <QDomDocument>

#include <errno.h>
#include <signal.h>

const QString   ProjectManager::unNamedProject = ProjectManager::tr( "Untitled Project" );
const QString   ProjectManager::unSavedProject = ProjectManager::tr( "Unsaved Project" );
const QString   ProjectManager::backupSuffix = "~";

ProjectManager::ProjectManager() : m_projectFile( NULL ), m_needSave( false )
{
    m_recentsProjects = VLMC_GET_STRINGLIST( "private/RecentsProjects" );
    //If the variable was empty, it will return a list with one empty string in it.
    m_recentsProjects.removeAll( "" );

    SettingValue    *fps = VLMC_CREATE_PROJECT_VAR( SettingValue::Double, "video/VLMCOutputFPS", 29.97,
                                QT_TRANSLATE_NOOP( "PreferenceWidget", "Output video FPS" ),
                                QT_TRANSLATE_NOOP( "PreferenceWidget", "Frame Per Second used when previewing and rendering the project" ),
                                                SettingValue::Clamped );
    fps->setLimits( 0.1, 120.0 );
    SettingValue    *width = VLMC_CREATE_PROJECT_VAR( SettingValue::Int, "video/VideoProjectWidth", 480,
                             QT_TRANSLATE_NOOP( "PreferenceWidget", "Video width" ),
                             QT_TRANSLATE_NOOP( "PreferenceWidget", "Width resolution of the output video" ),
                             SettingValue::Clamped | SettingValue::EightMultiple );
    width->setLimits( 32, 2048 );
    SettingValue    *height = VLMC_CREATE_PROJECT_VAR( SettingValue::Int, "video/VideoProjectHeight", 320,
                             QT_TRANSLATE_NOOP( "PreferenceWidget", "Video height" ),
                             QT_TRANSLATE_NOOP( "PreferenceWidget", "Height resolution of the output video" ),
                             SettingValue::Clamped | SettingValue::EightMultiple );
    height->setLimits( 32, 2048 );
    VLMC_CREATE_PROJECT_STRING( "video/AspectRatio", "16/9",
                                QT_TRANSLATE_NOOP("PreferenceWidget", "Video aspect ratio" ),
                                QT_TRANSLATE_NOOP("PreferenceWidget", "The rendered video aspect ratio" ) );
    SettingValue    *sampleRate = VLMC_CREATE_PROJECT_VAR( SettingValue::Double, "audio/AudioSampleRate", 44100,
                             QT_TRANSLATE_NOOP( "PreferenceWidget", "Audio samplerate" ),
                             QT_TRANSLATE_NOOP( "PreferenceWidget", "Output project audio samplerate" ),
                             SettingValue::Clamped );
    sampleRate->setLimits( 11025, 48000 );
    SettingValue    *audioChannel = VLMC_CREATE_PROJECT_VAR( SettingValue::Int, "audio/NbChannels", 2,
                                                             QT_TRANSLATE_NOOP("PreferenceWidget", "Audio channels" ),
                                                             QT_TRANSLATE_NOOP("PreferenceWidget", "Number of audio channels" ),
                                                             SettingValue::Clamped );
    audioChannel->setLimits( 2, 2 );
    VLMC_CREATE_PROJECT_VAR( SettingValue::String, "vlmc/ProjectName", unNamedProject,
                                QT_TRANSLATE_NOOP( "PreferenceWidget", "Project name" ),
                                QT_TRANSLATE_NOOP( "PreferenceWidget", "The project name" ),
                                SettingValue::NotEmpty );

    VLMC_CREATE_PRIVATE_PROJECT_STRING( "vlmc/Workspace", "" );

    //We have to wait for the library to be loaded before loading the workflow
    connect( Library::getInstance(), SIGNAL( projectLoaded() ), this, SLOT( loadWorkflow() ) );
    //Create the workspace:
    Workspace::getInstance();
}

ProjectManager::~ProjectManager()
{
    if ( m_projectFile != NULL )
        delete m_projectFile;
}


QStringList
ProjectManager::recentsProjects() const
{
    return m_recentsProjects;
}

void
ProjectManager::loadWorkflow()
{
    QDomElement     root = m_domDocument->documentElement();
    bool            savedState;

    MainWorkflow::getInstance()->loadProject( root );
    loadTimeline( root );
    if ( m_projectFile != NULL )
    {
        appendToRecentProject( projectName() );
        savedState = true;
    }
    else
        savedState = false;
    emit projectUpdated( projectName(), savedState );

    delete m_domDocument;
}

void
ProjectManager::loadProject( const QString& fileName )
{
    //FIXME:this is probably useless, as this is handled by the gui part now.
    //Don't print an error. The user most likely canceled the open project dialog.
    if ( fileName.isEmpty() == true )
        return ;

    if ( closeProject() == false )
        return ;
    m_projectFile = new QFile( fileName );
    QFileInfo       fInfo( fileName );
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
    SettingsManager::getInstance()->setValue( "vlmc/Workspace", fInfo.absolutePath(), SettingsManager::Project );
    Timeline::getInstance()->renderer()->loadProject( root );
    Library::getInstance()->loadProject( root );
}

void
ProjectManager::removeProject( const QString& project )
{
    // Remove all occurence of fileName
    m_recentsProjects.removeAll( project );

    SettingsManager::getInstance()->setValue( "private/RecentsProjects", m_recentsProjects, SettingsManager::Vlmc );
}

void
ProjectManager::__saveProject( const QString &fileName )
{
    QByteArray          projectString;

    QXmlStreamWriter    project( &projectString );

    project.setAutoFormatting( true );
    project.writeStartDocument();
    project.writeStartElement( "vlmc" );

    Library::getInstance()->saveProject( project );
    MainWorkflow::getInstance()->saveProject( project );
    Timeline::getInstance()->renderer()->saveProject( project );
    SettingsManager::getInstance()->save( project );
    saveTimeline( project );

    project.writeEndElement();
    project.writeEndDocument();

    QFile               file( fileName );
    file.open( QFile::WriteOnly );
    file.write( projectString );
}

void
ProjectManager::emergencyBackup()
{
    QString     name;

    if ( m_projectFile != NULL )
        name = createAutoSaveOutputFileName( m_projectFile->fileName() );
    else
       name = createAutoSaveOutputFileName( QDir::currentPath() + "/unsavedproject" );
    __saveProject( name );
    SettingsManager::getInstance()->setValue( "private/EmergencyBackup", name, SettingsManager::Vlmc );
}

bool
ProjectManager::isBackupFile( const QString& projectFile )
{
    return projectFile.endsWith( ProjectManager::backupSuffix );
}

QString
ProjectManager::createAutoSaveOutputFileName( const QString& baseName ) const
{
    return baseName + ProjectManager::backupSuffix;
}


void
ProjectManager::appendToRecentProject( const QString& projectName )
{
    // Append the item to the recents list
    m_recentsProjects.removeAll( projectName );
    m_recentsProjects.prepend( projectName );
    while ( m_recentsProjects.count() > 15 )
        m_recentsProjects.removeLast();

    SettingsManager::getInstance()->setValue( "private/RecentsProjects", m_recentsProjects, SettingsManager::Vlmc );
}

QString
ProjectManager::projectName() const
{
    //FIXME: Can this be true?
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
ProjectManager::saveAs( const QString &outputFileName )
{
    __saveProject( outputFileName );
    emit projectSaved();
    emit projectUpdated( projectName(), true );
}

bool
ProjectManager::loadEmergencyBackup()
{
    QString lastProject = VLMC_GET_STRING( "private/EmergencyBackup" );
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
    vlmcCritical() << tr( "Failed to load the project file: %1. Aborting." ).arg( reason );
    abort();
}

QString
ProjectManager::outputFileName() const
{
    return m_projectFile->fileName();
}
