/*****************************************************************************
 * Project.cpp: Handles all core project components
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

#include "Project.h"

#include <QFile>
#include <QFileInfo>
#include <QUndoStack>

#include "AutomaticBackup.h"
#include "Library/Library.h"
#include "Project.h"
#include "RecentProjects.h"
#include "Settings/Settings.h"
#include "Workflow/MainWorkflow.h"
#include "Workspace.h"

#include "Tools/VlmcDebug.h"

//FIXME: List of suspicious include from an architecture point of view:
#include "Renderer/WorkflowRenderer.h"

//FIXME: This shouldn't be here
#include "timeline/Timeline.h"

const QString   Project::unNamedProject = Project::tr( "Untitled Project" );
const QString   Project::backupSuffix = "~";

Project::Project()
    : m_projectFile( NULL )
    , m_isClean( true )
    , m_libraryCleanState( true )
{
    m_settings = new Settings( QString() );
    m_undoStack = new QUndoStack;
    m_workflow = new MainWorkflow();
    m_workspace = new Workspace( m_settings );
    m_library = new Library( m_workspace );

    connectComponents();
}

Project::~Project()
{
    delete m_projectFile;
    delete m_library;
    delete m_workspace;
    delete m_workflow;
    delete m_undoStack;
    delete m_settings;
}

Library*
Project::library()
{
    return m_library;
}

MainWorkflow*
Project::workflow()
{
    return m_workflow;
}

QUndoStack*
Project::undoStack()
{
    return m_undoStack;
}

Settings*
Project::settings()
{
    return m_settings;
}

Workspace*
Project::workspace()
{
    return m_workspace;
}

//////////////////////////////////////////////////////////////////////////////////////////

bool
Project::load( const QString& fileName )
{
    Project* self = getInstance();
    if ( fileName.isEmpty() == true )
        return false;
    QFile   projectFile( fileName );
    if ( projectFile.exists() == false )
        return false;
    if ( self->closeProject() == false )
        return false;
    Project::destroyInstance();

    // Now let's start over with a clean state.
    self = getInstance();
    self->loadProject( fileName );

    self->connectComponents();
    Core::getInstance()->automaticBackup()->setProject( self );
    Core::getInstance()->recentProjects()->setProject( self );
    return true;
}

bool
Project::create(const QString& projectName, const QString& projectPath )
{
    Project* self = getInstance();
    if ( self->closeProject() == false )
        return false;
    Project::destroyInstance();
    self = Project::getInstance();
    self->newProject( projectName, projectPath );

    self->connectComponents();
    Core::getInstance()->automaticBackup()->setProject( self );
    Core::getInstance()->recentProjects()->setProject( self );

    return true;
}

bool
Project::isProjectLoaded()
{
    return m_instance != NULL;
}

bool
Project::loadProject( const QString &fileName )
{
    QString fileToLoad = checkBackupFile( fileName );

    m_projectFile = new QFile( fileToLoad );
    m_domDocument = new QDomDocument;

    if ( m_projectFile->open( QFile::ReadOnly ) == false ||
         m_domDocument->setContent( m_projectFile ) == false )
    {
        vlmcCritical() << "Can't open project file" << m_projectFile->errorString();
        delete m_projectFile;
        m_projectFile = NULL;
        return false;
    }
    if ( fileToLoad.endsWith( Project::backupSuffix ) == true )
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
    return true;
}

void
Project::newProject( const QString &projectName, const QString &workspacePath )
{
    m_projectName = projectName;
    //Current project file has already been delete/nulled by closeProject()
    m_projectFile = new QFile( workspacePath + '/' + "project.vlmc" );
    save();
    emit projectLoaded( projectName, m_projectFile->fileName() );
}

void
Project::connectComponents()
{
    connect( m_library, SIGNAL( cleanStateChanged( bool ) ),
             this, SLOT( libraryCleanChanged( bool ) ) );
    connect( m_undoStack, SIGNAL( cleanChanged( bool ) ), this, SLOT( cleanChanged( bool ) ) );
    connect( this, SIGNAL( projectSaved() ), m_undoStack, SLOT( setClean() ) );
    //We have to wait for the library to be loaded before loading the workflow
    //FIXME
    //connect( Project::getInstance()->library(), SIGNAL( projectLoaded() ), this, SLOT( loadWorkflow() ) );
}

bool
Project::closeProject()
{
    Q_ASSERT( m_projectFile != NULL );
    if ( m_projectManagerUi != NULL )
    {
        IProjectUiCb::SaveMode mode = m_projectManagerUi->shouldSaveBeforeClose();
        if ( mode == IProjectUiCb::Cancel )
            return false;
        if ( mode == IProjectUiCb::Save )
            save();
    }
    delete m_projectFile;
    m_projectFile = NULL;
    m_isClean = true;
    m_projectName = QString();
    Project::getInstance()->undoStack()->clear();
    emit projectUpdated( name() );
    return true;
}

void
Project::save()
{
    saveProject( m_projectFile->fileName() );
}

void
Project::saveAs()
{
    if ( m_projectManagerUi == NULL )
        return ;
    const QString& fileName = m_projectManagerUi->getProjectFileDestination(
                m_settings->value( "vlmc/Workspace" )->get().toString() );
    if ( fileName.isEmpty() )
        return ;
    saveProject( fileName );
    emit projectUpdated( name() );
}

QString
Project::checkBackupFile(const QString& projectFile)
{
    QString backupFilename = projectFile + Project::backupSuffix;
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
Project::initSettings()
{
    SettingValue    *fps = m_settings->createVar( SettingValue::Double, "video/VLMCOutputFPS", 29.97,
                                QT_TRANSLATE_NOOP( "PreferenceWidget", "Output video FPS" ),
                                QT_TRANSLATE_NOOP( "PreferenceWidget", "Frame Per Second used when previewing and rendering the project" ),
                                                SettingValue::Clamped );
    fps->setLimits( 0.1, 120.0 );
    SettingValue    *width = m_settings->createVar( SettingValue::Int, "video/VideoProjectWidth", 480,
                             QT_TRANSLATE_NOOP( "PreferenceWidget", "Video width" ),
                             QT_TRANSLATE_NOOP( "PreferenceWidget", "Width resolution of the output video" ),
                             SettingValue::Clamped | SettingValue::EightMultiple );
    width->setLimits( 32, 2048 );
    SettingValue    *height = m_settings->createVar( SettingValue::Int, "video/VideoProjectHeight", 320,
                             QT_TRANSLATE_NOOP( "PreferenceWidget", "Video height" ),
                             QT_TRANSLATE_NOOP( "PreferenceWidget", "Height resolution of the output video" ),
                             SettingValue::Clamped | SettingValue::EightMultiple );
    height->setLimits( 32, 2048 );
    m_settings->createVar( SettingValue::String, "video/AspectRatio", "16/9",
                                QT_TRANSLATE_NOOP("PreferenceWidget", "Video aspect ratio" ),
                                QT_TRANSLATE_NOOP("PreferenceWidget", "The rendered video aspect ratio" ),
                                SettingValue::Nothing );
    SettingValue    *sampleRate = m_settings->createVar( SettingValue::Double, "audio/AudioSampleRate", 44100,
                             QT_TRANSLATE_NOOP( "PreferenceWidget", "Audio samplerate" ),
                             QT_TRANSLATE_NOOP( "PreferenceWidget", "Output project audio samplerate"),
                             SettingValue::Clamped );
    sampleRate->setLimits( 11025, 48000 );
    SettingValue    *audioChannel = m_settings->createVar( SettingValue::Int, "audio/NbChannels", 2,
                                                             QT_TRANSLATE_NOOP("PreferenceWidget", "Audio channels" ),
                                                             QT_TRANSLATE_NOOP("PreferenceWidget", "Number of audio channels" ),
                                                             SettingValue::Clamped );
    audioChannel->setLimits( 2, 2 );
    m_settings->createVar( SettingValue::String, "vlmc/ProjectName", unNamedProject,
                                QT_TRANSLATE_NOOP( "PreferenceWidget", "Project name" ),
                                QT_TRANSLATE_NOOP( "PreferenceWidget", "The project name" ),
                                SettingValue::NotEmpty );
    m_settings->watchValue( "vlmc/ProjectName", this, SLOT( projectNameChanged( QVariant ) ) );

    m_settings->createVar( SettingValue::String, "vlmc/Workspace", "", "", "", SettingValue::Private );
}

QString
Project::name()
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
Project::saveProject( const QString& fileName )
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
Project::emergencyBackup()
{
    Q_ASSERT( m_projectFile != NULL );
    const QString& name = m_projectFile->fileName() + Project::backupSuffix;
    saveProject( name );
    Core::getInstance()->settings()->setValue( "private/EmergencyBackup", name );
}

bool
Project::loadEmergencyBackup()
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

void
Project::cleanChanged( bool val )
{
    // This doesn't have to be different since we can force needSave = true when loading
    // a backup project file. This definitely needs testing though
    m_isClean = val;
    if ( m_libraryCleanState == m_isClean )
        emit cleanStateChanged( val );
}

void
Project::libraryCleanChanged(bool val)
{
    Q_ASSERT( m_libraryCleanState != val);
    m_libraryCleanState = val;
    if ( m_libraryCleanState == m_isClean )
        emit cleanStateChanged( val );
}

void
Project::projectNameChanged( const QVariant& name )
{
    m_projectName = name.toString();
    emit projectUpdated( m_projectName );
}

void
Project::loadWorkflow()
{
    QDomElement     root = m_domDocument->documentElement();

    Project::getInstance()->workflow()->loadProject( root );
    //FIXME: This was a no-op after a previous refactoring.
    //    loadTimeline( root );
    emit projectLoaded( name(), m_projectFile->fileName() );

    delete m_domDocument;
}


void
Project::autoSaveRequired()
{
    if ( m_projectFile == NULL )
        return ;
    saveProject( m_projectFile->fileName() + Project::backupSuffix );
}
