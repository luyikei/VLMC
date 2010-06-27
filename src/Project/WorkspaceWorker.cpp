/*****************************************************************************
 * WorkspaceWorker.cpp: The Workspace worker
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

#include "WorkspaceWorker.h"

#include "Media.h"
#include "SettingsManager.h"

#include <QFile>
#include <QFileInfo>

#include <QtDebug>

WorkspaceWorker::WorkspaceWorker( Media *media ) :
    m_media( media )
{
    connect( this, SIGNAL( finished() ), this, SLOT( deleteLater() ) );
}

void
WorkspaceWorker::run()
{
    QFile           file( m_media->fileInfo()->absoluteFilePath() );
    const QString   &projectPath = VLMC_PROJECT_GET_STRING( "general/Workspace" );
    const QString   dest = projectPath + '/' + m_media->fileInfo()->fileName();

    file.copy( m_media->fileInfo()->absoluteFilePath(), dest );
    qDebug() << "Media copied to:" << dest;
    emit copied( m_media, dest );
}
