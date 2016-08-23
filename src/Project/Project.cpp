/*****************************************************************************
 * Project.cpp: Handles all core project components
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "Project.h"

#include <QFile>
#include <QFileInfo>
#include <QTimer>


#include "Backend/IBackend.h"
#include "Backend/IProfile.h"
#include "Project.h"
#include "RecentProjects.h"
#include "Settings/Settings.h"
#include "Tools/VlmcDebug.h"

const QString   Project::unNamedProject = Project::tr( "Untitled Project" );
const QString   Project::backupSuffix = "~";

Project::Project( Settings* settings )
    : m_projectFile( nullptr )
    , m_isClean( true )
    , m_libraryCleanState( true )
    , m_timer( new QTimer( this ) )
    , m_settings( new Settings )
{
    initSettings();

    SettingValue    *automaticBackup = settings->createVar( SettingValue::Bool, "vlmc/AutomaticBackup", false,
                                     QT_TRANSLATE_NOOP( "PreferenceWidget", "Automatic save" ),
                                     QT_TRANSLATE_NOOP( "PreferenceWidget", "When this option is activated,"
                                                         "VLMC will automatically save your project "
                                                         "at a specified interval" ), SettingValue::Nothing );
    SettingValue    *automaticBackupInterval = settings->createVar( SettingValue::Int, "vlmc/AutomaticBackupInterval", 5,
                                    QT_TRANSLATE_NOOP( "PreferenceWidget", "Automatic save interval" ),
                                    QT_TRANSLATE_NOOP( "PreferenceWidget", "This is the interval that VLMC will wait "
                                                       "between two automatic save" ), SettingValue::Clamped );
    automaticBackupInterval->setLimits( 1, QVariant( QVariant::Invalid ) );

    connect( m_timer, &QTimer::timeout, this, &Project::autoSaveRequired );
    connect( this, &Project::destroyed, m_timer, &QTimer::stop );

    connect( automaticBackup, &SettingValue::changed,
             this, &Project::autoSaveEnabledChanged );
    connect( automaticBackupInterval, &SettingValue::changed,
             this, &Project::autoSaveIntervalChanged );
}

Project::~Project()
{
    delete m_projectFile;
    delete m_settings;
    delete m_timer;
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
            m_settings->setSettingsFile( backupFilename );
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
        m_settings->setSettingsFile( path );
    }

    m_settings->load();
    auto projectName = m_settings->value( "vlmc/ProjectName" )->get().toString();
    emit projectLoading( projectName );
    m_isClean = autoBackupFound == false;
    emit cleanStateChanged( m_isClean );
    if ( autoBackupFound == false )
        m_projectFile->close();
    emit projectLoaded( projectName, path );
    if ( outdatedBackupFound == true )
        emit outdatedBackupFileFound();
    if ( autoBackupFound == true )
        emit backupProjectLoaded();
    return true;
}

void
Project::save()
{
    Q_ASSERT( m_projectFile != nullptr );
    saveProject( m_projectFile->fileName() );
}

void
Project::saveAs( const QString& fileName )
{
    delete m_projectFile;
    m_projectFile = new QFile( fileName );
    saveProject( fileName );
}

void
Project::newProject( const QString& projectName, const QString& projectFilePath )
{
    closeProject();
    m_settings->setValue( "vlmc/ProjectName", projectName );
    m_projectFile = new QFile( projectFilePath );
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
    SettingValue    *aBitRate = m_settings->createVar( SettingValue::Int, "audio/AudioBitRate", 256,
                             QT_TRANSLATE_NOOP( "PreferenceWidget", "Audio bitrate" ),
                             QT_TRANSLATE_NOOP( "PreferenceWidget", "Output project audio bitrate (kbps)"),
                             SettingValue::Clamped );
    aBitRate->setLimits( 8, 512 );
    SettingValue    *vBitRate = m_settings->createVar( SettingValue::Int, "video/VideoBitRate", 4000,
                             QT_TRANSLATE_NOOP( "PreferenceWidget", "Video bitrate" ),
                             QT_TRANSLATE_NOOP( "PreferenceWidget", "Output project Video bitrate (kbps)"),
                             SettingValue::Clamped );
    vBitRate->setLimits( 8, 8192 );
    SettingValue    *audioChannel = m_settings->createVar( SettingValue::Int, "audio/NbChannels", 2,
                                                             QT_TRANSLATE_NOOP("PreferenceWidget", "Audio channels" ),
                                                             QT_TRANSLATE_NOOP("PreferenceWidget", "Number of audio channels" ),
                                                             SettingValue::Clamped );
    audioChannel->setLimits( 2, 2 );
    SettingValue    *pName = m_settings->createVar( SettingValue::String, "vlmc/ProjectName", unNamedProject,
                                    QT_TRANSLATE_NOOP( "PreferenceWidget", "Project name" ),
                                    QT_TRANSLATE_NOOP( "PreferenceWidget", "The project name" ),
                                    SettingValue::NotEmpty );
    connect( pName, SIGNAL( changed( QVariant ) ), this, SLOT( projectNameChanged( QVariant ) ) );
    connect( fps, &SettingValue::changed, this, [this]( const QVariant& var ){ emit fpsChanged( var.toDouble() ); } );
}

void
Project::saveProject( const QString& fileName )
{
    m_settings->setSettingsFile( fileName );
    m_settings->save();
    emit projectSaved();
}

void
Project::emergencyBackup()
{
    const QString& name = m_projectFile->fileName() + Project::backupSuffix;
    saveProject( name );
    Core::instance()->settings()->setValue( "private/EmergencyBackup", name );
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
    m_settings->restoreDefaultValues();
    emit projectClosed();
    delete m_projectFile;
    m_projectFile = nullptr;
}

bool
Project::hasProjectFile() const
{
    return m_projectFile != nullptr;
}

void
Project::removeBackupFile()
{
    QString         backupFilename = m_projectFile->fileName() + Project::backupSuffix;
    QFile           autoBackup( backupFilename );
    if ( autoBackup.exists() )
        autoBackup.remove();
}

QString
Project::name() const
{
    return m_settings->value( "vlmc/ProjectName" )->get().toString();
}

double
Project::fps() const
{
    return m_settings->value( "video/VLMCOutputFPS" )->get().toDouble();
}

unsigned int
Project::width() const
{
    return m_settings->value( "video/VideoProjectWidth" )->get().toUInt();
}

unsigned int
Project::height() const
{
    return m_settings->value( "video/VideoProjectHeight" )->get().toUInt();
}

QString
Project::aspectRatio() const
{
    return m_settings->value( "video/AspectRatio" )->get().toString();
}

unsigned int
Project::audioBitrate() const
{
    return m_settings->value( "audio/AudioBitRate" )->get().toUInt();
}

unsigned int
Project::videoBitrate() const
{
    return m_settings->value( "video/VideoBitRate" )->get().toUInt();
}

unsigned int
Project::sampleRate() const
{
    return m_settings->value( "audio/AudioSampleRate" )->get().toUInt();
}

unsigned int
Project::nbChannels() const
{
    return m_settings->value( "audio/NbChannels" )->get().toUInt();
}

QFile*
Project::emergencyBackupFile()
{
    const QString lastProject = Core::instance()->settings()->value( "private/EmergencyBackup" )->get().toString();
    if ( lastProject.isEmpty() == true )
        return nullptr;
    return new QFile( lastProject );
}

void
Project::cleanChanged( bool val )
{
    // This doesn't have to be different since we can force needSave = true when loading
    // a backup project file. This definitely needs testing though
    m_isClean = val;
    emit cleanStateChanged( m_libraryCleanState == true && m_isClean == true );
}

void
Project::libraryCleanChanged( bool val )
{
    Q_ASSERT( m_libraryCleanState != val);
    m_libraryCleanState = val;
    emit cleanStateChanged( m_libraryCleanState == true && m_isClean == true );
}

void
Project::autoSaveRequired()
{
    if ( m_projectFile == nullptr )
        return ;
    saveProject( m_projectFile->fileName() + Project::backupSuffix );
}


void
Project::autoSaveEnabledChanged( const QVariant& enabled )
{
    if ( enabled.toBool() == true )
    {
        int interval = Core::instance()->settings()->value( "vlmc/AutomaticBackupInterval" )->get().toInt();
        m_timer->start( interval * 1000 * 60 );
    }
    else
        m_timer->stop();
}

void
Project::autoSaveIntervalChanged( const QVariant& interval )
{
    bool enabled = Core::instance()->settings()->value( "vlmc/AutomaticBackup" )->get().toBool();

    if ( enabled == false )
        return ;
    m_timer->start( interval.toInt() * 1000 * 60 );
}
