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

#include "Project/ProjectManager.h"
#include "ProjectWizard.h"
#include "Settings/Settings.h"
#include "WelcomePage.h"
#include "OpenPage.h"
#include "GeneralPage.h"
#include "VideoPage.h"
#include "Tools/VlmcDebug.h"

ProjectWizard::ProjectWizard( ProjectManager* projectManager, QWidget* parent /*= NULL*/ )
    : QWizard( parent )
    , m_projectManager( projectManager )
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
        Settings* preferences = Core::getInstance()->settings();
        Settings* projectPreferences = Project::getInstance()->settings();
        m_projectManager->newProject( field( "projectName" ).toString(), field( "projectPath" ).toString() );

        preferences->setValue( "vlmc/DefaultProjectLocation", field( "workspace" ) );

        projectPreferences->setValue( "vlmc/Workspace", field( "projectPath" ) );
        projectPreferences->setValue( "video/VLMCOutputFPS", field( "fps" ) );
        projectPreferences->setValue( "video/VideoProjectHeight", field( "height" ) );
        projectPreferences->setValue( "video/VideoProjectWidth", field( "width" ) );
        projectPreferences->setValue( "video/AspectRatio", field( "aspectratio" ) );
        projectPreferences->setValue( "audio/AudioSampleRate", field( "samplerate" ) );
        projectPreferences->setValue( "audio/NbChannels", field( "samplerate" ) );
    }
    QDialog::accept();
}

void
ProjectWizard::reject()
{
    if ( m_projectManager->hasProjectLoaded() )
        return QWizard::reject();
    qApp->quit();
}
