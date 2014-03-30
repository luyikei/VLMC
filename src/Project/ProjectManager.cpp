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


#include <QDir>
#include <QDomDocument>
#include <QUndoStack>

#include <errno.h>
#include <signal.h>

#include "ProjectManager.h"
#include "Main/Core.h"
#include "Main/Project.h"
#include "Library/Library.h"
#include "Workflow/MainWorkflow.h"
#include "Settings/Settings.h"
#include "Gui/timeline/Timeline.h"
#include "Tools/VlmcDebug.h"
#include "Renderer/WorkflowRenderer.h"
#include "Project/Workspace.h"

const QString   ProjectManager::unNamedProject = ProjectManager::tr( "Untitled Project" );
const QString   ProjectManager::backupSuffix = "~";

ProjectManager::ProjectManager( Settings* projectSettings )
    : m_projectFile( NULL )
    , m_isClean( true )
    , m_libraryCleanState( true )
    , m_projectSettings( projectSettings )
{
    initSettings();
    //We have to wait for the library to be loaded before loading the workflow
    //FIXME
    //connect( Project::getInstance()->library(), SIGNAL( projectLoaded() ), this, SLOT( loadWorkflow() ) );
}

ProjectManager::~ProjectManager()
{
    delete m_projectFile;
}

void
ProjectManager::setProjectManagerUi( IProjectManagerUiCb *projectManagerUi )
{
    m_projectManagerUi = projectManagerUi;
}

void
ProjectManager::loadWorkflow()
{
    QDomElement     root = m_domDocument->documentElement();

    Project::getInstance()->workflow()->loadProject( root );
    loadTimeline( root );
    emit projectLoaded( projectName(), m_projectFile->fileName() );

    delete m_domDocument;
}

void
ProjectManager::saveProject( const QString& fileName )
{
    QByteArray          projectString;

    QXmlStreamWriter    project( &projectString );

    project.setAutoFormatting( true );
    project.writeStartDocument();
    project.writeStartElement( "vlmc" );

    Project::getInstance()->library()->saveProject( project );
    Project::getInstance()->workflow()->saveProject( project );
    Timeline::getInstance()->renderer()->saveProject( project );
    Core::getInstance()->settings()->save( project );
    Timeline::getInstance()->save( project );

    project.writeEndElement();
    project.writeEndDocument();

    QFile   projectFile( fileName );
    projectFile.open( QFile::WriteOnly );
    projectFile.write( projectString );
    emit projectSaved();
}

void
ProjectManager::emergencyBackup()
{
    Q_ASSERT( m_projectFile != NULL );
    const QString& name = m_projectFile->fileName() + ProjectManager::backupSuffix;
    saveProject( name );
    Core::getInstance()->settings()->setValue( "private/EmergencyBackup", name );
}

bool
ProjectManager::hasProjectLoaded() const
{
    return m_projectFile != NULL;
}

QString
ProjectManager::projectName() const
{
    if ( m_projectName.isEmpty() == true )
    {
        Q_ASSERT( m_projectFile != NULL );

        QFileInfo       fInfo( *m_projectFile );
        return fInfo.baseName();
    }
    return m_projectName;
}

void
ProjectManager::save()
{
    saveProject( m_projectFile->fileName() );
}

void
ProjectManager::saveAs()
{
    if ( m_projectManagerUi == NULL )
        return ;
    const QString& fileName = m_projectManagerUi->getProjectFileDestination(
                m_projectSettings->value( "vlmc/Workspace" )->get().toString() );
    if ( fileName.isEmpty() )
        return ;
    saveProject( fileName );
    emit projectUpdated( projectName() );
}

bool
ProjectManager::loadEmergencyBackup()
{
    const QString lastProject = Core::getInstance()->settings()->value( "private/EmergencyBackup" )->get().toString();
    if ( QFile::exists( lastProject ) == true )
    {
        loadProject(  lastProject );
        m_isClean = false;
        return true;
    }
    return false;
}

QString
ProjectManager::checkBackupFile(const QString& projectFile)
{
    QString backupFilename = projectFile + ProjectManager::backupSuffix;
    QFile   autoBackup( backupFilename );
    if ( autoBackup.exists() == true )
    {
        QFileInfo       projectFileInfo( projectFile );
        QFileInfo       autobackupFileInfo( autoBackup );

        if ( autobackupFileInfo.lastModified() > projectFileInfo.lastModified() )
        {
            if ( m_projectManagerUi != NULL && m_projectManagerUi->shouldLoadBackupFile() )
                return backupFilename;
        }
        else
        {
            if ( m_projectManagerUi != NULL && m_projectManagerUi->shouldDeleteOutdatedBackupFile() )
                autoBackup.remove();
        }
    }
    return projectFile;
}

void
ProjectManager::initSettings()
{
    SettingValue    *fps = m_projectSettings->createVar( SettingValue::Double, "video/VLMCOutputFPS", 29.97,
                                QT_TRANSLATE_NOOP( "PreferenceWidget", "Output video FPS" ),
                                QT_TRANSLATE_NOOP( "PreferenceWidget", "Frame Per Second used when previewing and rendering the project" ),
                                                SettingValue::Clamped );
    fps->setLimits( 0.1, 120.0 );
    SettingValue    *width = m_projectSettings->createVar( SettingValue::Int, "video/VideoProjectWidth", 480,
                             QT_TRANSLATE_NOOP( "PreferenceWidget", "Video width" ),
                             QT_TRANSLATE_NOOP( "PreferenceWidget", "Width resolution of the output video" ),
                             SettingValue::Clamped | SettingValue::EightMultiple );
    width->setLimits( 32, 2048 );
    SettingValue    *height = m_projectSettings->createVar( SettingValue::Int, "video/VideoProjectHeight", 320,
                             QT_TRANSLATE_NOOP( "PreferenceWidget", "Video height" ),
                             QT_TRANSLATE_NOOP( "PreferenceWidget", "Height resolution of the output video" ),
                             SettingValue::Clamped | SettingValue::EightMultiple );
    height->setLimits( 32, 2048 );
    m_projectSettings->createVar( SettingValue::String, "video/AspectRatio", "16/9",
                                QT_TRANSLATE_NOOP("PreferenceWidget", "Video aspect ratio" ),
                                QT_TRANSLATE_NOOP("PreferenceWidget", "The rendered video aspect ratio" ),
                                SettingValue::Nothing );
    SettingValue    *sampleRate = m_projectSettings->createVar( SettingValue::Double, "audio/AudioSampleRate", 44100,
                             QT_TRANSLATE_NOOP( "PreferenceWidget", "Audio samplerate" ),
                             QT_TRANSLATE_NOOP( "PreferenceWidget", "Output project audio samplerate"),
                             SettingValue::Clamped );
    sampleRate->setLimits( 11025, 48000 );
    SettingValue    *audioChannel = m_projectSettings->createVar( SettingValue::Int, "audio/NbChannels", 2,
                                                             QT_TRANSLATE_NOOP("PreferenceWidget", "Audio channels" ),
                                                             QT_TRANSLATE_NOOP("PreferenceWidget", "Number of audio channels" ),
                                                             SettingValue::Clamped );
    audioChannel->setLimits( 2, 2 );
    m_projectSettings->createVar( SettingValue::String, "vlmc/ProjectName", unNamedProject,
                                QT_TRANSLATE_NOOP( "PreferenceWidget", "Project name" ),
                                QT_TRANSLATE_NOOP( "PreferenceWidget", "The project name" ),
                                SettingValue::NotEmpty );
    m_projectSettings->watchValue( "vlmc/ProjectName", this, SLOT( projectNameChanged( QVariant ) ) );

    m_projectSettings->createVar( SettingValue::String, "vlmc/Workspace", "", "", "", SettingValue::Private );
}

void
ProjectManager::cleanChanged( bool val )
{
    // This doesn't have to be different since we can force needSave = true when loading
    // a backup project file. This definitely needs testing though
    m_isClean = val;
    if ( m_libraryCleanState == m_isClean )
        emit cleanStateChanged( val );
}

void
ProjectManager::libraryCleanChanged(bool val)
{
    Q_ASSERT( m_libraryCleanState != val);
    m_libraryCleanState = val;
    if ( m_libraryCleanState == m_isClean )
        emit cleanStateChanged( val );
}

void
ProjectManager::projectNameChanged( const QVariant& name )
{
    m_projectName = name.toString();
    emit projectUpdated( m_projectName );
}

void
ProjectManager::newProject( const QString &projectName, const QString &workspacePath )
{
    if ( closeProject() == false )
        return ;
    m_projectName = projectName;
    //Current project file has already been delete/nulled by closeProject()
    m_projectFile = new QFile( workspacePath + '/' + "project.vlmc" );
    save();
    emit projectLoaded( projectName, m_projectFile->fileName() );
}

void
ProjectManager::loadProject( const QString &fileName )
{
    if ( fileName.isEmpty() == true )
        return ;
    QFile   projectFile( fileName );
    //If for some reason this happens... better safe than sorry
    if ( projectFile.exists() == false )
        return ;

    QString fileToLoad = checkBackupFile( fileName );

    if ( closeProject() == false )
        return ;

    m_projectFile = new QFile( fileToLoad );
    m_domDocument = new QDomDocument;

    if ( m_projectFile->open( QFile::ReadOnly ) == false ||
         m_domDocument->setContent( m_projectFile ) == false )
    {
        vlmcCritical() << "Can't open project file" << m_projectFile->errorString();
        delete m_projectFile;
        m_projectFile = NULL;
        return ;
    }
    if ( fileToLoad.endsWith( ProjectManager::backupSuffix ) == true )
    {
        // Delete the project file representation, so the next time the user
        // saves its project, vlmc will ask her where to save it.
        delete m_projectFile;
        m_projectFile = NULL;
        m_isClean = false;
    }

    QDomElement     root = m_domDocument->documentElement();

    //Load settings first, as it contains some informations about the workspace.
    Project::getInstance()->settings()->setSettingsFile( fileName );
    Project::getInstance()->settings()->load();
    Timeline::getInstance()->renderer()->loadProject( root );
    Project::getInstance()->library()->loadProject( root );
    m_projectFile->close();
}

void
ProjectManager::autoSaveRequired()
{
    if ( m_projectFile == NULL )
        return ;
    saveProject( m_projectFile->fileName() + ProjectManager::backupSuffix );
}

bool
ProjectManager::closeProject()
{
    Q_ASSERT( m_projectFile != NULL );
    if ( m_projectManagerUi != NULL )
    {
        IProjectManagerUiCb::SaveMode mode = m_projectManagerUi->shouldSaveBeforeClose();
        if ( mode == IProjectManagerUiCb::Cancel )
            return false;
        if ( mode == IProjectManagerUiCb::Save )
            save();
    }
    delete m_projectFile;
    m_projectFile = NULL;
    m_isClean = true;
    m_projectName = QString();
    Project::getInstance()->undoStack()->clear();
    emit projectUpdated( projectName() );
    return true;
}
