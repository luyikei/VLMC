/*****************************************************************************
 * FirstLaunchWizard.cpp: First launch wizard
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

#include "FirstLaunchWizard.h"
#include "WorkspaceLocation.h"
#include "FirstLaunchPage.h"
#include "MediaLibraryDirs.h"
#include "Done.h"
#include "Settings/Settings.h"

FirstLaunchWizard::FirstLaunchWizard( QWidget* parent )
    : QWizard( parent )
{
#ifndef Q_OS_MAC
    setWizardStyle( QWizard::ModernStyle );
#endif

    addPage( new FirstLaunchPage( this ) );
    addPage( new WorkspaceLocation( this ) );
    addPage( new MediaLibraryDirs( this ) );
    addPage( new Done( this ) );
}

bool
FirstLaunchWizard::shouldRun()
{
    // Try not to leak private variables outside of where they are actually used.
    return VLMC_GET_BOOL( "private/FirstLaunchDone" ) == false;
}

void
FirstLaunchWizard::accept()
{
    auto workspaceLocation = field( "workspaceLocation" );
    auto mlDirs = field( "mlDirs" );
    Q_ASSERT( workspaceLocation.toString().isEmpty() == false );
    Core::instance()->settings()->setValue( "vlmc/WorkspaceLocation", workspaceLocation );
    Core::instance()->settings()->setValue( "private/FirstLaunchDone", true );
    Core::instance()->settings()->setValue( "vlmc/mlDirs", mlDirs );
    QDialog::accept();
}
