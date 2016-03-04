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
#include <QDomDocument>

#include "AutomaticBackup.h"
#include "Backend/IBackend.h"
#include "Project.h"
#include "ProjectCallbacks.h"
#include "RecentProjects.h"
#include "Settings/Settings.h"
#include "Tools/VlmcDebug.h"

//FIXME: This is required to save for now, but it feels wrong
#include "Workflow/MainWorkflow.h"
#include "Library/Library.h"
#include "Renderer/WorkflowRenderer.h"
#include "timeline/Timeline.h"

const QString   Project::unNamedProject = Project::tr( "Untitled Project" );
const QString   Project::backupSuffix = "~";

Project::Project()
    : m_projectFile( nullptr )
    , m_isClean( true )
    , m_libraryCleanState( true )
    , m_projectManagerUi( nullptr )
{
    m_settings = new Settings( QString() );
    initSettings();
}

Project::~Project()
{
    delete m_projectFile;
    delete m_settings;
}

Settings*
Project::settings()
{
    return m_settings;
}

//////////////////////////////////////////////////////////////////////////////////////////

bool
Project::load( const QString& path )
{
    closeProject();

    QString         backupFilename = path + Project::backupSuffix;
    QFile           autoBackup( backupFilename );
    QDomDocument    doc;
    bool            autoBackupFound = false;
    bool            outdatedBackupFound = false;

    // Always consider the actual project as the project file
    m_projectFile = new QFile( path );
    if ( m_projectFile->exists() == false )
    {
        delete m_projectFile;
        return false;
    }

    if ( autoBackup.exists() == true )
    {
        QFileInfo       projectFileInfo( *m_projectFile );
        QFileInfo       autobackupFileInfo( autoBackup );

        if ( autobackupFileInfo.lastModified() < projectFileInfo.lastModified() )
            outdatedBackupFound = true;
        else
        {
            if ( autoBackup.open( QFile::ReadOnly ) == false )
            {
                vlmcCritical() << "Can't open project file" << backupFilename;
                delete m_projectFile;
                m_projectFile = nullptr;
                return false;
            }
            autoBackupFound = true;
            doc.setContent( &autoBackup );
        }
    }
    else
    {
        if ( m_projectFile->open( QFile::ReadOnly ) == false )
        {
            delete m_projectFile;
            m_projectFile = nullptr;
            return false;
        }
        doc.setContent( m_projectFile );
    }

    m_settings->load( doc );
    emit projectLoading( m_projectName );
    m_isClean = autoBackupFound == false;
    emit cleanStateChanged( m_isClean );
    if ( autoBackupFound == false )
        m_projectFile->close();
    emit projectLoaded( m_projectName );
    if ( outdatedBackupFound )
        emit outdatedBackupFileFound( backupFilename );
    return true;
}

void
Project::save()
{
    Q_ASSERT( m_projectFile != NULL );
    saveProject( m_projectFile->fileName() );
}

void
Project::saveAs( const QString& fileName )
{
    delete m_projectFile;
    m_projectFile = new QFile( fileName );
    saveProject( fileName );
    emit projectUpdated( name() );
}

void
Project::newProject( const QString& projectName, const QString& projectPath )
{
    closeProject();
    m_settings->setValue( "vlmc/ProjectName", projectName );
    m_projectFile = new QFile( projectPath );
    save();
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
	SettingValue* pName = m_settings->createVar( SettingValue::String, "vlmc/ProjectName", unNamedProject,
									QT_TRANSLATE_NOOP( "PreferenceWidget", "Project name" ),
									QT_TRANSLATE_NOOP( "PreferenceWidget", "The project name" ),
									SettingValue::NotEmpty );
    // Use direct connection to have the project name stored in m_projectName as soon as we
    // are done loading the settings.
    connect( pName, SIGNAL( changed( QVariant ) ),
             this, SLOT( projectNameChanged( QVariant ) ), Qt::DirectConnection );
}

const QString&
Project::name()
{
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

    m_settings->save( project );
    Core::getInstance()->workflow()->save( project );
    Core::getInstance()->library()->save( project );
    Core::getInstance()->workflowRenderer()->save( project );
    //FIXME: Timeline configuration isn't saved anymore.

    project.writeEndElement();
    project.writeEndDocument();

    // We are not necessarily saving the project to the "main" project file here
    // so we don't use m_projectFile
    QFile   projectFile( fileName );
    projectFile.open( QFile::WriteOnly );
    projectFile.write( projectString );
    emit projectSaved();
}

void
Project::emergencyBackup()
{
    const QString& name = m_projectFile->fileName() + Project::backupSuffix;
    saveProject( name );
    Core::getInstance()->settings()->setValue( "private/EmergencyBackup", name );
}

bool
Project::isClean() const
{
    return m_isClean;
}

void
Project::closeProject()
{
    if ( m_projectFile == nullptr )
        return;
    // FIXME: Restore settings to their default value
    emit projectClosed();
    delete m_projectFile;
    m_projectFile = nullptr;
    m_projectName.clear();
}

bool
Project::hasProjectFile() const
{
    return m_projectFile != nullptr;
}

QFile* Project::emergencyBackupFile()
{
    const QString lastProject = Core::getInstance()->settings()->value( "private/EmergencyBackup" )->get().toString();
    if ( lastProject.isEmpty() == true )
        return NULL;
    return new QFile( lastProject );
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
Project::autoSaveRequired()
{
    if ( m_projectFile == NULL )
        return ;
    saveProject( m_projectFile->fileName() + Project::backupSuffix );
}
