/*****************************************************************************
 * WorkspaceLocation.cpp: First launch wizard
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

#include "WorkspaceLocation.h"
#include "ui/WorkspaceLocation.h"

#include <QFileDialog>

WorkspaceLocation::WorkspaceLocation( QWidget *parent )
    : QWizardPage( parent )
    , m_ui( new Ui::WorkspaceLocation )
{
    m_ui->setupUi(this);

    setTitle( tr( "Workspace location" ) );

    connect( m_ui->browseButton, SIGNAL( clicked( bool ) ),
            this, SLOT( browse() ) );
    registerField( "workspaceLocation*", m_ui->workspaceLocation );
}

void
WorkspaceLocation::browse()
{
    auto location = QFileDialog::getExistingDirectory( this, tr( "Select a workspace location" ) );
    if ( location.isEmpty() == true )
        return;
    m_ui->workspaceLocation->setText( location );
}

WorkspaceLocation::~WorkspaceLocation()
{
    delete m_ui;
}
