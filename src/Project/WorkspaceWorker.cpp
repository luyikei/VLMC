/*****************************************************************************
 * WorkspaceWorker.cpp: The Workspace worker
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

#include "Project/WorkspaceWorker.h"

#include "Media/Media.h"
#include "Settings/Settings.h"
#include "Tools/VlmcDebug.h"

#include <cerrno>
#include <unistd.h>

#include <QFile>
#include <QFileInfo>

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
    errno = 0;
    if ( link( m_media->fileInfo()->absoluteFilePath().toUtf8().constData(),
          m_dest.toUtf8().constData() ) < 0 )
    {
        vlmcDebug() << "Can't create hard link:" << strerror(errno) << "falling back to"
                " hard copy mode.";
    }
    else
    {
        vlmcDebug() << "Media hard linked to:" << m_dest;
        hardLinkOk = true;
    }
#endif

    if ( hardLinkOk == false )
    {
        QFile::copy( m_media->fileInfo()->absoluteFilePath(), m_dest );
        vlmcDebug() << "Media copied to:" << m_dest;
    }
    emit copied( m_media, m_dest );
}
