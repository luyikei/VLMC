/*****************************************************************************
 * Workspace.h: Workspace management
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

#ifndef WORKSPACE_H
#define WORKSPACE_H

#include <QObject>

#include "Singleton.hpp"
class   Clip;
class   Media;

class Workspace : public QObject, public Singleton<Workspace>
{
    Q_OBJECT

    public:
        static const QString        workspacePrefix;

        static bool                 isInProjectDir( const Media* media );
        static QString              pathInProjectDir( const Media* media );
    private:
        Workspace();
        ~Workspace(){}

        void    copyToWorkspace( Media* media );
    public slots:
        void    clipLoaded( Clip* clip );
    private slots:
        void    copyTerminated( Media* media, QString dest );

    friend class    Singleton<Workspace>;
};

#endif // WORKSPACE_H
