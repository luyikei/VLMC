/*****************************************************************************
 * Library.cpp: Multimedia library
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

/** \file
  * This file the library contains class implementation.
  * It's the the backend part of the Library widget of vlmc.
  * It can load and unload Medias (Medias.h/Media.cpp)
  * It can load and unload Clips (Clip.h/Clip.cpp)
  */

#include "Clip.h"
#include "Library.h"
#include "Media.h"
#include "MetaDataManager.h"
#include "SettingsManager.h"
#include "Workspace.h"

#include <QtDebug>
#include <QDomElement>
#include <QHash>
#include <QUuid>

Library::Library() : m_cleanState( true )
{
}

void
Library::loadProject( const QDomElement& doc )
{
    const QDomElement   medias = doc.firstChildElement( "medias" );

    if ( medias.isNull() == true )
        return ;

    //Add a virtual media, which represents all the clip.
    //This avoid emitting projectLoaded(); before all the clip are actually loaded.
    m_nbMediaToLoad = 1;
    QDomElement media = medias.firstChildElement();
    while ( media.isNull() == false )
    {
        if ( media.hasAttribute( "mrl" ) == true )
        {
            QString mrl = media.attribute( "mrl" );

            //If in workspace: compute the path in workspace
            if ( mrl.startsWith( Workspace::workspacePrefix ) == true )
            {
                //Transforming the workspace://[path] into [project-path]/[path]
                QString     projectPath = VLMC_PROJECT_GET_STRING( "general/Workspace" );
                mrl = projectPath + mrl.mid( Workspace::workspacePrefix.length() );
            }
            Media*  m = addMedia( mrl );
            if ( m == NULL )
                qWarning() << "Failed to load media" << mrl << "when loading project.";
            else
                m_nbMediaToLoad.fetchAndAddAcquire( 1 );
        }
        media = media.nextSiblingElement();
    }
    const QDomElement   clips = doc.firstChildElement( "clips" );
    if ( clips.isNull() == true )
        return ;
    load( clips, this );
    mediaLoaded( NULL );
    //Mark the state as clean, as we just loaded a project. Otherwise, a media
    //loading triggers a setCleanState(false) which makes sense when modifying
    //project, but not here.
    setCleanState(true);
}

void
Library::saveProject( QXmlStreamWriter& project )
{
    QHash<QUuid, Clip*>::const_iterator     it = m_clips.begin();
    QHash<QUuid, Clip*>::const_iterator     end = m_clips.end();

    project.writeStartElement( "medias" );
    while ( it != end )
    {
        it.value()->getMedia()->save( project );
        ++it;
    }
    project.writeEndElement();
    project.writeStartElement( "clips" );
    save( project );
    project.writeEndElement();
    setCleanState( true );
}

void
Library::mediaLoaded( const Media* media )
{
    if ( media != NULL )
    {
        disconnect( media, SIGNAL( metaDataComputed( const Media* ) ),
                 this, SLOT( mediaLoaded( const Media* ) ) );
    }
    m_nbMediaToLoad.fetchAndAddAcquire( -1 );
    if ( m_nbMediaToLoad == 0 )
        emit projectLoaded();
}

void
Library::addMedia( Media* media )
{
    setCleanState( false );
    MediaContainer::addMedia( media );
}

Media*
Library::addMedia( const QFileInfo &fileInfo )
{
    Media* media = MediaContainer::addMedia( fileInfo );
    if ( media != NULL )
    {
        setCleanState( false );
        connect( media, SIGNAL( metaDataComputed( const Media* ) ),
                 this, SLOT( mediaLoaded( const Media* ) ), Qt::QueuedConnection );
        m_medias[fileInfo.absoluteFilePath()] = media;
        media->computeMetadata();
    }
    return media;
}

bool
Library::addClip( Clip *clip )
{
    bool    ret = MediaContainer::addClip( clip );
    if ( ret != false )
        setCleanState( false );
    return ret;
}

bool
Library::isInCleanState() const
{
    return m_cleanState;
}

void
Library::setCleanState( bool newState )
{
    if ( newState != m_cleanState )
    {
        m_cleanState = newState;
        emit cleanStateChanged( newState );
    }
}
