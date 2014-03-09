/*****************************************************************************
 * Workspace.cpp: Workspace management
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

#include "Workspace.h"

#include "Main/Project.h"
#include "Media/Clip.h"
#include "Library/Library.h"
#include "Media/Media.h"
#include "Settings/Settings.h"
#include "Tools/VlmcDebug.h"

#include "Project/WorkspaceWorker.h"

#include <QMutex>

#ifdef WITH_GUI
# include "Gui/widgets/NotificationZone.h"
# include <QMessageBox>
#endif

const QString   Workspace::workspacePrefix = "workspace://";

Workspace::Workspace() : m_copyInProgress( false )
{
    m_mediasToCopyMutex = new QMutex;
#ifdef WITH_GUI
    connect( this, SIGNAL( notify( QString ) ),
             NotificationZone::getInstance(), SLOT( notify( QString ) ) );
#endif
}

Workspace::~Workspace()
{
    delete m_mediasToCopyMutex;
}

bool
Workspace::copyToWorkspace( Media *media )
{
    if ( VLMC_PROJECT_GET_STRING("vlmc/Workspace").length() == 0 )
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
        if ( media->isInWorkspace() == false )
        {
            startCopyWorker( media );
        }
    }
    return true;
}

void
Workspace::startCopyWorker( Media *media )
{
    const QString   &projectPath = VLMC_PROJECT_GET_STRING( "vlmc/Workspace" );
    const QString   dest = projectPath + '/' + media->fileInfo()->fileName();

    if ( QFile::exists( dest ) == true )
    {
#ifdef WITH_GUI
        QMessageBox::StandardButton b =
                QMessageBox::question( NULL, tr( "File already exists!" ),
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
    if ( clip->getMedia()->isInWorkspace() == true )
        return ;
    copyToWorkspace( clip->getMedia() );
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
            if ( toCopy->isInWorkspace() == false )
            {
                startCopyWorker( toCopy );
                break ;
            }
        }
    }
    else
        m_copyInProgress = false;
}

bool
Workspace::isInProjectDir( const QFileInfo &fInfo )
{
    const QString       projectDir = VLMC_PROJECT_GET_STRING( "vlmc/Workspace" );

    return ( projectDir.length() > 0 && fInfo.absolutePath().startsWith( projectDir ) );
}

bool
Workspace::isInProjectDir( const QString &path )
{
    QFileInfo           fInfo( path );

    return isInProjectDir( fInfo );
}

bool
Workspace::isInProjectDir(const Media *media)
{
    return isInProjectDir( *(media->fileInfo() ) );
}

QString
Workspace::pathInProjectDir( const Media *media )
{
    const QString      projectDir = VLMC_PROJECT_GET_STRING( "vlmc/Workspace" );

    return ( media->fileInfo()->absoluteFilePath().mid( projectDir.length() ) );
}

void
Workspace::copyAllToWorkspace()
{
    if ( Project::getInstance()->library()->m_clips.size() == 0 )
        return ;
    QHash<QUuid, Clip*>::iterator    it = Project::getInstance()->library()->m_clips.begin();
    QHash<QUuid, Clip*>::iterator    ite = Project::getInstance()->library()->m_clips.end();

    {
        QMutexLocker    lock( m_mediasToCopyMutex );
        while ( it != ite )
        {
            m_mediasToCopy.enqueue( it.value()->getMedia() );
            ++it;
        }
    }
    copyToWorkspace( m_mediasToCopy.dequeue() );
}
