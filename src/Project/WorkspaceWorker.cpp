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

#include <cerrno>
#include <unistd.h>

#include <QFile>
#include <QFileInfo>

#include <QtDebug>

WorkspaceWorker::WorkspaceWorker( Media *media, const QString &dest ) :
    m_media( media ),
    m_dest( dest )
{
    connect( this, SIGNAL( finished() ), this, SLOT( deleteLater() ) );
}

void
WorkspaceWorker::run()
{
    bool            hardLinkOk = false;

#ifdef Q_OS_UNIX
    if ( link( m_media->fileInfo()->absoluteFilePath().toUtf8().constData(),
          m_dest.toUtf8().constData() ) < 0 )
    {
        qDebug() << "Can't create hard link:" << strerror(errno) << "falling back to"
                " hard copy mode.";
    }
    else
    {
        qDebug() << "Media hard linked to:" << m_dest;
        hardLinkOk = true;
    }
#endif

    if ( hardLinkOk == false )
    {
        QFile           file( m_media->fileInfo()->absoluteFilePath() );

        file.copy( m_media->fileInfo()->absoluteFilePath(), m_dest );
        qDebug() << "Media copied to:" << m_dest;
    }
    emit copied( m_media, m_dest );
}
