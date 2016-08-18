/*****************************************************************************
 * Workspace.h: Workspace management
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

#ifndef WORKSPACE_H
#define WORKSPACE_H

#include <QObject>

#include "Tools/ErrorHandler.h"

class   QFileInfo;

class   Clip;
class   Media;
class   Settings;

class Workspace : public QObject, public ErrorHandler
{
    Q_OBJECT

    public:
        static const QString        workspacePrefix;

        Workspace( Settings* settings );
        bool                        isInWorkspace( const QString &path );
        bool                        isInWorkspace( const Media *media );

    private:
        bool                        isInWorkspace( const QFileInfo &fInfo );
    private:
        QString                     m_workspaceDir;

    private slots:
        void                        workspaceChanged( const QVariant& newWorkspace );

    signals:
        void                        notify( QString );
};

#endif // WORKSPACE_H
