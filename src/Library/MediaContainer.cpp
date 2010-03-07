/*****************************************************************************
 * MediaContainer.cpp: Implements the library basics
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

#include <QHash>
#include <QUuid>

#include "Clip.h"
#include "MediaContainer.h"
#include "Media.h"
#include "MetaDataManager.h"

Media*
MediaContainer::media( const QUuid& uuid )
{
    QHash<QUuid, Media*>::const_iterator   it = m_medias.find( uuid );
    if ( it == m_medias.end() )
        return NULL;
    return *it;
}

Clip*
MediaContainer::clip( const QUuid& uuid )
{
    Media*  m = media( uuid );
    if ( m != NULL )
        return m->baseClip();

    foreach( m, m_medias.values() )
    {
        if ( m != NULL && m->clips().contains( uuid ) )
            return m->clip( uuid );
    }
    return NULL;
}

Clip*
MediaContainer::clip( const QUuid& mediaUuid, const QUuid& clipUuid )
{
    if ( m_medias.contains( mediaUuid ) )
    {
        if ( m_medias.value( mediaUuid )->clips().contains( clipUuid ) )
            return m_medias.value( mediaUuid )->clip( clipUuid );
        else
            m_medias.value( mediaUuid )->baseClip();
    }
    return NULL;
}

void
MediaContainer::removingMediaAsked( const QUuid& uuid )
{
    deleteMedia( uuid );
    emit mediaRemoved( uuid );
}

void
MediaContainer::deleteMedia( const QUuid& uuid )
{
    if ( m_medias.contains( uuid ) )
        delete m_medias.take( uuid );
}

bool
MediaContainer::addMedia( const QFileInfo& fileInfo, const QString& uuid )
{
    if ( QFile::exists( fileInfo.absoluteFilePath() ) == false )
        return false;
    Media* media = new Media( fileInfo.filePath(), uuid );

    foreach( Media* it, m_medias.values() )
    {
        if ( it->fileInfo()->filePath() == media->fileInfo()->filePath() )
        {
            delete media;
            return false;
        }
    }
    MetaDataManager::getInstance()->computeMediaMetadata( media );
    addMedia( media );
    return true;
}


void
MediaContainer::addMedia( Media *media )
{
    m_medias[media->baseClip()->uuid()] = media;
    emit newMediaLoaded( media );
}

bool
MediaContainer::mediaAlreadyLoaded( const QFileInfo& fileInfo )
{
    foreach( Media* media, m_medias.values() )
    {
        if ( media->fileInfo()->filePath() == fileInfo.filePath() )
            return true;
    }
    return false;
}

void
MediaContainer::clear()
{
    QHash<QUuid, Media*>::iterator  it = m_medias.begin();
    QHash<QUuid, Media*>::iterator  end = m_medias.end();

    while ( it != end )
    {
        emit mediaRemoved( it.key() );
        delete it.value();
        ++it;
    }
    m_medias.clear();
}

void
MediaContainer::removeAll()
{
    QHash<QUuid, Media*>::iterator  it = m_medias.begin();
    QHash<QUuid, Media*>::iterator  end = m_medias.end();

    while ( it != end )
    {
        emit mediaRemoved( it.key() );
        ++it;
    }
    m_medias.clear();
}

void
MediaContainer::removeClip( const QUuid &uuid )
{
    foreach( Media* media, m_medias.values() )
    {
        if ( media->clip( uuid ) )
        {
            media->removeClip( uuid );
            return ;
        }
    }
}

void
MediaContainer::removeClip( const QUuid& mediaId, const QUuid& clipId )
{
    Media*  med = 0;
    if ( m_medias.contains( mediaId ) )
        med = m_medias[mediaId];
    else
        return;

    if ( med->clips().contains( clipId ) )
        med->removeClip( clipId );
}

const QHash<QUuid, Media*>&
MediaContainer::medias() const
{
    return m_medias;
}
