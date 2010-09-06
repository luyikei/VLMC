/*****************************************************************************
 * main.cpp: VLMC main for non GUI mode
 *****************************************************************************
 * Copyright (C) 2008-2009 the VLMC team
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

#include "config.h"
#include "ConsoleRenderer.h"
#include "ProjectManager.h"
#include "WorkflowFileRenderer.h"

#include <QCoreApplication>
#include <QtDebug>
#include <QMetaType>
#include <QVariant>

/**
 *  VLMC Entry point
 *  \brief this is the VLMC entry point
 *  \param argc
 *  \param argv
 *  \return Return value of vlmc
 */
int
VLMCmain( int argc, char **argv )
{
    QCoreApplication app( argc, argv );
    app.setApplicationName( "vlmc" );
    app.setOrganizationName( "vlmc" );
    app.setOrganizationDomain( "vlmc.org" );
    app.setApplicationVersion( PROJECT_VERSION );

    qRegisterMetaType<Workflow::TrackType>( "Workflow::TrackType" );
    qRegisterMetaType<Vlmc::FrameChangedReason>( "Vlmc::FrameChangedReason" );
    qRegisterMetaType<QVariant>( "QVariant" );

    if ( app.arguments().count() < 3 )
    {
        qCritical() << "Usage: ./vlmc project.vlmc output_file";
        return 1;
    }
    ProjectManager  *pm = ProjectManager::getInstance();
    ConsoleRenderer renderer;

    QCoreApplication::connect( pm, SIGNAL( projectLoaded() ), &renderer, SLOT( startRender() ) );
    pm->loadProject( app.arguments()[1] );
    return app.exec();
}

