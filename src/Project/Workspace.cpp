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

#include "Project/WorkspaceWorker.h"

#include <QMutex>

#ifdef HAVE_GUI
# include "Gui/widgets/NotificationZone.h"
# include <QMessageBox>
#endif

const QString   Workspace::workspacePrefix = "workspace://";

Workspace::Workspace(Settings *settings)
    : m_copyInProgress( false )
{
    SettingValue* workspaceDir = settings->createVar( SettingValue::String, "vlmc/Workspace", "",
                                                      "", "", SettingValue::Private );
    connect(workspaceDir, SIGNAL( changed( QVariant ) ),
            this, SLOT( workspaceChanged( QVariant ) ) );
    // Wait for the SettingValue to be loaded.
    m_mediasToCopyMutex = new QMutex;
#ifdef HAVE_GUI
    connect( this, SIGNAL( notify( QString ) ),
             NotificationZone::instance(), SLOT( notify( QString ) ) );
#endif
}

Workspace::~Workspace()
{
    delete m_mediasToCopyMutex;
}

bool
Workspace::copyToWorkspace( Media *media )
{
    if ( m_workspaceDir.isEmpty() )
    {
        setError( "There is no current workspace. Please create a project first.");
        return false;
    }
    QMutexLocker    lock( m_mediasToCopyMutex );

    if ( m_copyInProgress == true )
    {
        m_mediasToCopy.enqueue( media );
    }
    else
    {
        vlmcDebug() << "Copying media:" << media->fileInfo()->absoluteFilePath() << "to workspace.";
        m_copyInProgress = true;
        Q_ASSERT( this->isInWorkspace( media ) == false );
        startCopyWorker( media );
    }
    return true;
}

void
Workspace::startCopyWorker( Media *media )
{
    const QString   dest = m_workspaceDir + '/' + media->fileInfo()->fileName();

    if ( QFile::exists( dest ) == true )
    {
#ifdef HAVE_GUI
        QMessageBox::StandardButton b =
                QMessageBox::question( nullptr, tr( "File already exists!" ),
                                       tr( "A file with the same name already exists, do you want to "
                                           "overwrite it?" ),
                                       QMessageBox::Yes | QMessageBox::No,
                                       QMessageBox::No );
        if ( b == QMessageBox::No )
            copyTerminated( media, dest );
#else
        copyTerminated( media, dest );
#endif
    }
    WorkspaceWorker *worker = new WorkspaceWorker( media, dest );
    //This one is direct connected since the thread is terminated just after emitting the signal.
    connect( worker, SIGNAL( copied( Media*, QString ) ),
             this, SLOT( copyTerminated( Media*, QString ) ), Qt::DirectConnection );
    worker->start();
}

void
Workspace::clipLoaded( Clip *clip )
{
    //Don't bother if the clip is a subclip.
    if ( clip->isRootClip() == false )
        return ;
    //If already in workspace : well...
    if ( isInWorkspace( clip->media() ) == true )
        return ;
    copyToWorkspace( clip->media() );
}

void
Workspace::copyTerminated( Media *media, QString dest )
{
    emit notify( tr( "Workspace: " ) + media->fileInfo()->fileName() + tr( " copied to " ) + dest );

    media->setFilePath( dest );
    media->disconnect( this );

    QMutexLocker    lock( m_mediasToCopyMutex );
    if ( m_mediasToCopy.size() > 0 )
    {
        while ( m_mediasToCopy.size() > 0 )
        {
            Media   *toCopy = m_mediasToCopy.dequeue();
            if ( isInWorkspace( media ) == true )
            {
                startCopyWorker( toCopy );
                break ;
            }
        }
    }
    else
        m_copyInProgress = false;
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
    return isInWorkspace( *(media->fileInfo() ) );
}
