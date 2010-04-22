/*****************************************************************************
 * GUIProjectManager.cpp: Handle the GUI part of the project managing
 *****************************************************************************
 * Copyright (C) 2008-2010 VideoLAN
 *
 * Authors: Hugo Beauzée-Luyssen <beauze.h@gmail.com>
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

#include "GuiProjectManager.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>

#include "Library.h"
#include "MainWorkflow.h"
#include "SettingsManager.h"
#include "Timeline.h"

GUIProjectManager::GUIProjectManager()
{
    connect( this, SIGNAL( projectClosed() ), Library::getInstance(), SLOT( clear() ) );
    connect( this, SIGNAL( projectClosed() ), MainWorkflow::getInstance(), SLOT( clear() ) );

    //Automatic save part :
    m_timer = new QTimer( this );
    connect( m_timer, SIGNAL( timeout() ), this, SLOT( autoSaveRequired() ) );
    VLMC_CREATE_PREFERENCE_BOOL( "general/AutomaticBackup", false,
                                 QT_TRANSLATE_NOOP( "PreferenceWidget", "Automatic save" ),
                                 QT_TRANSLATE_NOOP( "PreferenceWidget", "When this option is activated,"
                                             "VLMC will automatically save your project "
                                             "at a specified interval" ) );
    SettingsManager::getInstance()->watchValue( "general/AutomaticBackup", this,
                                                SLOT( automaticSaveEnabledChanged(QVariant) ),
                                                SettingsManager::Vlmc,
                                                Qt::QueuedConnection );
    VLMC_CREATE_PREFERENCE_INT( "general/AutomaticBackupInterval", 5,
                                QT_TRANSLATE_NOOP( "PreferenceWidget", "Automatic save interval" ),
                                QT_TRANSLATE_NOOP( "PreferenceWidget", "This is the interval that VLMC will wait "
                                            "between two automatic save" ) );
    SettingsManager::getInstance()->watchValue( "general/AutomaticBackupInterval", this,
                                                SLOT( automaticSaveIntervalChanged(QVariant) ),
                                                SettingsManager::Vlmc,
                                                Qt::QueuedConnection );
    automaticSaveEnabledChanged( VLMC_GET_BOOL( "general/AutomaticBackup" ) );
    SettingsManager::getInstance()->watchValue( "general/ProjectName", this,
                                                SLOT(projectNameChanged(QVariant) ),
                                                SettingsManager::Project );
}

bool
GUIProjectManager::askForSaveIfModified()
{
    if ( m_needSave == true )
    {
        QMessageBox msgBox;
        msgBox.setText( tr( "The project has been modified." ) );
        msgBox.setInformativeText( tr( "Do you want to save it?" ) );
        msgBox.setStandardButtons( QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel );
        msgBox.setDefaultButton( QMessageBox::Save );
        int     ret = msgBox.exec();

        switch ( ret )
        {
            case QMessageBox::Save:
                saveProject();
                break ;
            case QMessageBox::Discard:
                break ;
            case QMessageBox::Cancel:
            default:
                return false ;
        }
    }
    return true;
}

QString
GUIProjectManager::acquireProjectFileName()
{
    QString fileName =
            QFileDialog::getOpenFileName( NULL, "Enter the output file name",
                                          VLMC_PROJECT_GET_STRING( "general/VLMCWorkspace" ),
                                          "VLMC project file(*.vlmc)" );
    return fileName;
}

bool
GUIProjectManager::createNewProjectFile( bool saveAs )
{
    if ( m_projectFile == NULL || saveAs == true )
    {
        QString outputFileName =
            QFileDialog::getSaveFileName( NULL, "Enter the output file name",
                                          VLMC_PROJECT_GET_STRING( "general/VLMCWorkspace" ),
                                          "VLMC project file(*.vlmc)" );
        if ( outputFileName.length() == 0 )
            return false;
        if ( m_projectFile != NULL )
            delete m_projectFile;
        if ( outputFileName.endsWith( ".vlmc" ) == false )
            outputFileName += ".vlmc";
        m_projectFile = new QFile( outputFileName );
        appendToRecentProject( outputFileName );
        emit projectUpdated( projectName(), true );
    }
    return true;
}

void
GUIProjectManager::saveProject( bool saveAs /*= true*/ )
{
    //If the project is still unsaved, or if we want to
    //save the project with a new name
    if ( createNewProjectFile( saveAs ) == false )
        return ;
    ProjectManager::saveProject( saveAs );
}

bool
GUIProjectManager::closeProject()
{
    if ( askForSaveIfModified() == false )
        return false;
    return ProjectManager::closeProject();
}

void
GUIProjectManager::newProject( const QString &projectName )
{
    if ( closeProject() == false )
        return ;
    m_projectName = projectName;
    emit projectUpdated( this->projectName(), false );
}


void
GUIProjectManager::autoSaveRequired()
{
    if ( m_projectFile == NULL )
        return ;
    saveProject( false );
}

void
GUIProjectManager::automaticSaveEnabledChanged( const QVariant& val )
{
    bool    enabled = val.toBool();

    if ( enabled == true )
    {
        int interval = VLMC_GET_INT( "general/AutomaticBackupInterval" );
        m_timer->start( interval * 1000 * 60 );
    }
    else
        m_timer->stop();
}

void
GUIProjectManager::automaticSaveIntervalChanged( const QVariant& val )
{
    bool enabled = VLMC_GET_BOOL( "general/AutomaticBackup" );

    if ( enabled == false )
        return ;
    m_timer->start( val.toInt() * 1000 * 60 );
}

bool
GUIProjectManager::needSave() const
{
    return m_needSave;
}

void
GUIProjectManager::cleanChanged( bool val )
{
    m_needSave = !val;
    emit projectUpdated( projectName(), val );
}


void
GUIProjectManager::projectNameChanged( const QVariant& name )
{
    m_projectName = name.toString();
    emit projectUpdated( m_projectName, !m_needSave );
}

void
GUIProjectManager::failedToLoad( const QString &reason ) const
{
    QMessageBox::warning( NULL, tr( "Failed to load project file" ), reason );
}

void
GUIProjectManager::saveTimeline( QXmlStreamWriter &project )
{
    Timeline::getInstance()->save( project );
}
