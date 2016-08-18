/*****************************************************************************
 * Workspace.cpp: Workspace management
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

#include "Workspace.h"

#include "Project/Project.h"
#include "Media/Clip.h"
#include "Library/Library.h"
#include "Media/Media.h"
#include "Settings/Settings.h"
#include "Tools/VlmcDebug.h"

#include <QFileInfo>

const QString   Workspace::workspacePrefix = "workspace://";

Workspace::Workspace(Settings *settings)
{
    SettingValue* workspaceDir = settings->createVar( SettingValue::String, "vlmc/Workspace", "",
                                                      "", "", SettingValue::Private );
    connect(workspaceDir, SIGNAL( changed( QVariant ) ),
            this, SLOT( workspaceChanged( QVariant ) ) );
}

void
Workspace::workspaceChanged(const QVariant &newWorkspace)
{
    m_workspaceDir = newWorkspace.toString();
}

bool
Workspace::isInWorkspace( const QFileInfo &fInfo )
{
    return ( m_workspaceDir.length() > 0 && fInfo.absolutePath().startsWith( m_workspaceDir ) );
}

bool
Workspace::isInWorkspace( const QString &path )
{
    QFileInfo           fInfo( path );

    return isInWorkspace( fInfo );
}

bool
Workspace::isInWorkspace(const Media *media)
{
    return isInWorkspace( media->mrl() );
}
