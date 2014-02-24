/*****************************************************************************
 * Workspace.h: Workspace management
 *****************************************************************************
 * Copyright (C) 2008-2014 VideoLAN
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

#include "Singleton.hpp"

#include "ErrorHandler.h"

#include <QMutex>
#include <QObject>
#include <QQueue>

class   Clip;
class   Media;

class   QFileInfo;

class Workspace : public QObject, public Singleton<Workspace>, public ErrorHandler
{
    Q_OBJECT

    public:
        static const QString        workspacePrefix;

        static bool                 isInProjectDir( const QString &path );
        static bool                 isInProjectDir( const QFileInfo &fInfo );
        static bool                 isInProjectDir( const Media *media );
        static QString              pathInProjectDir( const Media* media );

        bool                        copyToWorkspace( Media* media );
        void                        copyAllToWorkspace();
    private:
        Workspace();
        ~Workspace();
        void                        startCopyWorker( Media *media );
    private:
        QQueue<Media*>              m_mediasToCopy;
        QMutex                      *m_mediasToCopyMutex;
        bool                        m_copyInProgress;

    public slots:
        void                        clipLoaded( Clip* clip );
    private slots:
        void                        copyTerminated( Media* media, QString dest );

    signals:
        void                        notify( QString );

    friend class    Singleton<Workspace>;
};

#endif // WORKSPACE_H
