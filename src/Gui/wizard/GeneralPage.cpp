/*****************************************************************************
 * SettingsManager.cpp: Backend settings manager
 *****************************************************************************
 * Copyright (C) 2008-2014 VideoLAN
 *
 * Authors: Ludovic Fauvet <etix@l0cal.com>
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

#include <QFileDialog>
#include <QDir>
#include <QMessageBox>

#include "GeneralPage.h"
#include "ProjectWizard.h"
#include "Settings/Settings.h"
#include "Project/ProjectManager.h"

GeneralPage::GeneralPage( QWidget *parent ) :
    QWizardPage( parent )
{
    ui.setupUi( this );

    setTitle( tr( "New project wizard" ) );
    setSubTitle( tr( "Set General options" ) );

    // Create palettes
    pValid = pInvalid = palette();
    pInvalid.setColor( QPalette::Text, QColor( 215, 30, 30 ) );

    connect( ui.pushButtonBrowse, SIGNAL( clicked() ),
             this, SLOT( openWorkspaceDirectory() ) );
    connect( ui.lineEditName, SIGNAL( textChanged(QString) ),
             this, SLOT( updateProjectLocation() ) );
    connect( ui.lineEditWorkspace, SIGNAL( textChanged(QString) ),
             this, SLOT( updateProjectLocation() ) );

    registerField( "projectName*", ui.lineEditName );
    registerField( "projectPath*", ui.lineEditProjectLocation );
    registerField( "workspace*", ui.lineEditWorkspace );
}

void
GeneralPage::changeEvent( QEvent *e )
{
    QWizardPage::changeEvent( e );
    switch ( e->type() )
    {
    case QEvent::LanguageChange:
        ui.retranslateUi( this );
        break;
    default:
        break;
    }
}

int
GeneralPage::nextId() const
{
    return ProjectWizard::Page_Video;
}

void
GeneralPage::initializePage()
{
    //Since this is a new project, it will be unnamed
    QString     projectName = ProjectManager::unNamedProject;
    ui.lineEditName->setText( projectName );

    //fetching the global workspace path
    QString     workspacePath = VLMC_GET_STRING( "vlmc/DefaultProjectLocation" );
    ui.lineEditWorkspace->setText( workspacePath );

    //Reinit description field
    ui.textEditDescription->clear();

    updateProjectLocation();
}

void GeneralPage::cleanupPage()
{
}

bool
GeneralPage::validatePage()
{
    if ( m_valid == false )
        return false;
    const QString       &defaultProjectName = ProjectManager::unNamedProject;
    if ( ui.lineEditName->text().isEmpty() ||
         ui.lineEditName->text() == defaultProjectName )
    {
        QMessageBox::information( this, tr( "Form is incomplete" ),
                                  tr( "The project name must be filled." ) );
        ui.lineEditName->setFocus();
        return false;
    }
    if ( ui.lineEditWorkspace->text().isEmpty() )
    {
        QMessageBox::information( this, tr( "Form is incomplete" ),
                                  tr( "The workspace location must be set." ) );
        ui.lineEditWorkspace->setFocus();
        return false;
    }

    //Create the project directory in the workspace dir.
    QString     projectPath = ui.lineEditName->text().replace( ' ', '_' );
    QDir        workspaceDir( ui.lineEditWorkspace->text() );

    if ( workspaceDir.exists( projectPath ) == false )
        workspaceDir.mkdir( projectPath );
    return true;
}

void
GeneralPage::openWorkspaceDirectory()
{
    QString     workspace = QFileDialog::getExistingDirectory( this,
                                                           "Choose a workspace directory",
                                                           QDir::homePath() );
    if ( workspace.isEmpty() )
        return;
    ui.lineEditWorkspace->setText( workspace );
}

void
GeneralPage::updateProjectLocation()
{
    QString     workspacePath = ui.lineEditWorkspace->text();
    if ( workspacePath.isEmpty() )
    {
        ui.lineEditProjectLocation->setText( tr( "Missing workspace location" ) );
        setValidity( false );
        return ;
    }
    else
    {
        QString     pName = ui.lineEditName->text().replace( ' ', '_' );
        QDir        workspaceDir( workspacePath );
        QDir        projectDir( QString( "%1/%2" ).arg( workspacePath, pName ) );

        ui.lineEditProjectLocation->setText( projectDir.absolutePath() );

        if ( workspaceDir.isRelative() )
        {
            ui.lineEditProjectLocation->setText( tr( "Invalid workspace location" ) );
            setValidity( false );
            return ;
        }

        if ( !workspaceDir.exists() )
            setValidity( false );
        else //Invalidate the path if the project directory already exists
            setValidity( !projectDir.exists() );
    }
}

void
GeneralPage::setValidity( bool status )
{
    if ( status == true )
        ui.lineEditProjectLocation->setPalette( pValid );
    else
        ui.lineEditProjectLocation->setPalette( pInvalid );
    m_valid = status;
}
