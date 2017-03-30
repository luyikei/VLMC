/*****************************************************************************
 * WelcomePage.cpp: Wizard welcome page
 *****************************************************************************
 * Copyright (C) 2008-2016 VideoLAN
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "WelcomePage.h"

#include "Project/RecentProjects.h"
#include "Settings/Settings.h"

#include <QFileDialog>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QFileInfo>

QString* WelcomePage::m_projectPath = nullptr;

WelcomePage::WelcomePage( QWidget* parent )
    : QWizardPage( parent )
{
    m_ui.setupUi( this );

    setTitle( tr( "Project wizard" ) );
    setSubTitle( tr( "Open or create a project" ) );

    connect( m_ui.openPushButton, SIGNAL( clicked() ),
             this, SLOT( loadProject() ) );
    connect( m_ui.removeProjectButton, SIGNAL( clicked() ),
             this, SLOT( removeProject() ) );
    connect( m_ui.projectsListWidget, SIGNAL( itemPressed(QListWidgetItem*) ),
             this, SLOT( selectOpenRadio() ) );
    connect( m_ui.projectsListWidget, SIGNAL( itemDoubleClicked(QListWidgetItem*) ),
             this, SLOT( projectDoubleClicked(QListWidgetItem*) ) );
    connect( m_ui.projectsListWidget, &QListWidget::itemSelectionChanged, this, &WelcomePage::itemSelectionChanged);

    registerField( "loadProject", m_ui.projectsListWidget );
    m_projectPath = new QString();
}

WelcomePage::~WelcomePage()
{
    delete m_projectPath;
}

void
WelcomePage::changeEvent( QEvent *e )
{
    switch ( e->type() )
    {
    case QEvent::LanguageChange:
        m_ui.retranslateUi( this );
        break;
    default:
        break;
    }
}

int
WelcomePage::nextId() const
{
    if ( m_ui.createRadioButton->isChecked() )
        return ProjectWizard::Page_General;
    else
        return ProjectWizard::Page_Open;
}

void
WelcomePage::initializePage()
{
    m_ui.createRadioButton->setChecked( true );
    loadRecentsProjects();
}

bool
WelcomePage::validatePage()
{
    if ( m_ui.openRadioButton->isChecked() )
    {
        if ( m_projectPath->isEmpty() == true )
        {
            QMessageBox::information( this, tr( "Sorry" ),
                                      tr( "You first need to select a project from "
                                      "the list.\nThen click next to continue..." ) );
            return false;
        }
        else if ( QFile::exists( *m_projectPath ) == false )
        {
            QMessageBox::warning( this, tr( "Warning" ),
                                  QStringLiteral( "%1\n%2" )
                                  .arg( tr( "Sorry, we couldn't find your file. Was it moved, renamed, or deleted?" ) )
                                  .arg( *m_projectPath ) );
            return false;
        }
        return true;
    }
    return true;
}

void
WelcomePage::cleanupPage()
{

}

void
WelcomePage::loadRecentsProjects()
{
    m_ui.projectsListWidget->clear();

    for ( const auto& var : Core::instance()->recentProjects()->toVariant().toList() )
    {
        const auto& name = var.toMap()["name"].toString();
        const auto& filePath = var.toMap()["file"].toString();
        QListWidgetItem* item = new QListWidgetItem( name );
        item->setData( FilePath, filePath );
        m_ui.projectsListWidget->addItem( item );
    }
}

void
WelcomePage::loadProject()
{
    QString projectPath =
            QFileDialog::getOpenFileName( nullptr, tr( "Select a project file" ),
                                          VLMC_GET_STRING( "vlmc/WorkspaceLocation" ),
                                          tr( "VLMC project file(*.vlmc)" ) );

    if ( projectPath.isEmpty() ) return;

    if ( !QFile(projectPath).exists() )
    {
        QMessageBox::warning(this, tr("Invalid project file path"),
                             tr("Please use an existing project file."));
        return;
    }

    selectOpenRadio();

    m_ui.projectsListWidget->clearSelection();
    m_ui.projectsListWidget->clearFocus();

    setProjectPath( projectPath );
    if ( wizard() )
        wizard()->next();
}

void
WelcomePage::removeProject()
{
    QList<QListWidgetItem*> selected = m_ui.projectsListWidget->selectedItems();
    if ( selected.isEmpty() )
        return;

    const QString projectPath = selected.at( 0 )->data( FilePath ).toString();
    if ( projectPath.isEmpty() )
        return;

    Core::instance()->recentProjects()->remove( projectPath );
    loadRecentsProjects(); // Reload recent projects
}

void
WelcomePage::selectOpenRadio()
{
    m_ui.openRadioButton->setChecked( true );
    m_ui.removeProjectButton->setEnabled( true );
}

void
WelcomePage::itemSelectionChanged()
{
    if ( m_ui.projectsListWidget->selectedItems().count() == 0 )
        setProjectPath( "" );
    else
    {
        QList<QListWidgetItem*> selected = m_ui.projectsListWidget->selectedItems();
        setProjectPath( selected.at( 0 )->data( FilePath ).toString() );
    }
}

void
WelcomePage::projectDoubleClicked( QListWidgetItem* item )
{
    Q_UNUSED( item );
    if ( wizard() )
        wizard()->next();
}

QString
WelcomePage::projectPath()
{
    return *m_projectPath;
}

void
WelcomePage::setProjectPath( const QString& path )
{
    m_projectPath->clear();
    m_projectPath->append( path );
}
