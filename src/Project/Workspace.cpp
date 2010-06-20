/*****************************************************************************
 * Workspace.cpp: Workspace management
 *****************************************************************************
 * Copyright (C) 2008-2010 VideoLAN
 *
 * Authors: Hugo Beauzee-Luyssen <hugo@vlmc.org>
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

#include "Workspace.h"

#include "Clip.h"
#include "Library.h"
#include "Media.h"
#include "Project/WorkspaceWorker.h"
#include "SettingsManager.h"

#include <QtDebug>

const QString   Workspace::workspacePrefix = "workspace://";

Workspace::Workspace()
{
//    connect( Library::getInstance(), SIGNAL( newClipLoaded( Clip* ) ),
//             this, SLOT( clipLoaded( Clip* ) ) );
}

void
Workspace::copyToWorkspace( Media *media )
{
    qDebug() << "Copying media:" << media->fileInfo()->absoluteFilePath() << "to workspace.";
    if ( media->isInWorkspace() == false )
    {
        WorkspaceWorker *worker = new WorkspaceWorker( media );
        //This one is direct connected since the thread is terminated just after emitting the signal.
        connect( worker, SIGNAL( copied( Media*, QString ) ),
                 this, SLOT( copyTerminated( Media*, QString ) ), Qt::DirectConnection );
        worker->start();
    }
}

void
Workspace::clipLoaded( Clip *clip )
{
    //Don't bother if the clip is a subclip.
    if ( clip->isRootClip() == false )
        return ;
    //If already in workspace : well...
    if ( clip->getMedia()->isInWorkspace() == true )
        return ;
    copyToWorkspace( clip->getMedia() );
}

void
Workspace::copyTerminated( Media *media, QString dest )
{
    media->setFilePath( dest );
}

bool
Workspace::isInProjectDir( const Media *media )
{
    const QString      projectDir = VLMC_PROJECT_GET_STRING( "general/ProjectDir" );

    return ( media->fileInfo()->absoluteFilePath().startsWith( projectDir ) );
}

QString
Workspace::pathInProjectDir( const Media *media )
{
    const QString      projectDir = VLMC_PROJECT_GET_STRING( "general/ProjectDir" );

    return ( media->fileInfo()->absoluteFilePath().mid( projectDir.length() ) );
}

