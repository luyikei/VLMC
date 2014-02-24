/*****************************************************************************
 * ProjectWizard.cpp: Project wizard
 *****************************************************************************
 * Copyright (C) 2008-2014 VideoLAN
 *
 * Authors: Clement CHAVANCE <kinder@vlmc.org>
 *          Ludovic Fauvet <etix@l0cal.com>
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

#include <QString>
#include <QMessageBox>
#include <QWizardPage>
#include "project/GuiProjectManager.h"
#include "ProjectWizard.h"
#include "SettingsManager.h"
#include "WelcomePage.h"
#include "OpenPage.h"
#include "GeneralPage.h"
#include "VideoPage.h"

ProjectWizard::ProjectWizard( QWidget* parent )
    : QWizard( parent )
{
    // Create Wizard

#ifndef Q_OS_MAC
    setWizardStyle( QWizard::ModernStyle );
#endif

    QPixmap logo = QPixmap( ":/images/vlmc" )
                   .scaledToHeight( 50, Qt::SmoothTransformation );

    setPixmap( QWizard::LogoPixmap, logo );
    setPixmap( QWizard::WatermarkPixmap, QPixmap( ":/images/wizard_watermark" ) );
    setOption( QWizard::IndependentPages );
    setWindowTitle( tr( "Project wizard" ) );

    // Show and connect the help button
    setOption( HaveHelpButton, true );
    connect( this, SIGNAL( helpRequested() ), this, SLOT( showHelp() ) );

    // Create pages

    QWizardPage* welcomePage = new WelcomePage( this );
    QWizardPage* generalPage = new GeneralPage( this );
    QWizardPage* videoPage = new VideoPage( this );
    QWizardPage* openPage = new OpenPage( this );

    setPage( Page_Welcome, welcomePage );
    setPage( Page_General, generalPage );
    setPage( Page_Video, videoPage );
    setPage( Page_Open, openPage );

    // Set the start page
    setStartId( Page_Welcome );
}

ProjectWizard::~ProjectWizard()
{
}

void
ProjectWizard::showHelp()
{
    QString message;

    switch ( currentId() )
    {
    case Page_Welcome:
        message = tr( "Choose the appropriate action then click Next to continue." );
        break;
    default:
        message = tr( "This help is likely not to be of any help." );
    }

    QMessageBox::information( this, tr( "Project wizard help" ), message );
}

void
ProjectWizard::accept()
{
    //This intend to check if the user opened a project, or created one.
    //If he was creating a project, the current page will be the video/audio settings one.
    if ( currentId() == Page_Video )
    {
        SettingsManager *sManager = SettingsManager::getInstance();
        GUIProjectManager::getInstance()->newProject( field( "projectName" ).toString(), field( "projectPath" ).toString() );
        //Save the project workspace
        sManager->setValue( "vlmc/Workspace", field( "projectPath" ), SettingsManager::Project );
        //And the default vlmc workspace
        sManager->setValue( "vlmc/DefaultProjectLocation", field( "workspace" ), SettingsManager::Vlmc );

        sManager->setValue( "video/VLMCOutputFPS", field( "fps" ), SettingsManager::Project );
        sManager->setValue( "video/VideoProjectHeight", field( "height" ), SettingsManager::Project );
        sManager->setValue( "video/VideoProjectWidth", field( "width" ), SettingsManager::Project );
        sManager->setValue( "video/AspectRatio", field( "aspectratio" ), SettingsManager::Project );
        sManager->setValue( "audio/AudioSampleRate", field( "samplerate" ), SettingsManager::Project );
        sManager->setValue( "audio/NbChannels", field( "samplerate" ), SettingsManager::Project );
    }
    QDialog::accept();
}

void
ProjectWizard::reject()
{
    QDialog::reject();
}
