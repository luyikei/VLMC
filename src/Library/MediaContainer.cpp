/*****************************************************************************
 * MediaContainer.cpp: Implements the library basics
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

#include <QHash>
#include <QUuid>

#include "Library.h"
#include "MediaContainer.h"
#include "Media/Clip.h"
#include "Media/Media.h"
#include "EffectsEngine/EffectHelper.h"
#include "Settings/Settings.h"
#include "Tools/VlmcDebug.h"
#include "Project/Workspace.h"

MediaContainer::MediaContainer( Clip* parent /*= nullptr*/ ) : m_parent( parent )
{
}

MediaContainer::~MediaContainer()
{
    foreach ( Clip* c, m_clips.values() )
        delete c;
    m_clips.clear();
}

Clip*
MediaContainer::clip( const QUuid& uuid )
{
    return clip( uuid.toString() );
}

Clip*
MediaContainer::clip( const QString &uuid )
{
    for ( const auto& clip : m_clips )
        if ( clip->uuid().toString() == uuid )
            return clip;
        else
        {
            auto c = clip->mediaContainer()->clip( uuid );
            if ( c != nullptr )
                return c;
        }
    return nullptr;
}

void
MediaContainer::addMedia( Media *media )
{
    m_clips[media->baseClip()->uuid()] = media->baseClip();
    emit newClipLoaded( media->baseClip() );
}

Media*
MediaContainer::addMedia( const QFileInfo& fileInfo )
{
    if ( QFile::exists( fileInfo.absoluteFilePath() ) == false )
    {
        vlmcCritical() << "Can't add" << fileInfo.absoluteFilePath() << ": File not found";
        return nullptr;
    }
    foreach( Clip* it, m_clips.values() )
    {
        if ( (*it->media()->fileInfo()) == fileInfo )
        {
            vlmcWarning() << "Ignoring aleady imported media" << fileInfo.absolutePath();
            return nullptr;
        }
    }
    Media* media = new Media( fileInfo.filePath() );
    return media;
}

bool
MediaContainer::mediaAlreadyLoaded( const QFileInfo& fileInfo )
{
    foreach( Clip* clip, m_clips.values() )
    {
        if ( clip->media()->fileInfo()->filePath() == fileInfo.filePath() )
            return true;
    }
    return false;
}

bool
MediaContainer::addClip( Clip* clip )
{
    foreach ( Clip* c, m_clips.values() )
    {
        if ( clip->uuid() == c->uuid() ||
             ( clip->media()->fileInfo() == c->media()->fileInfo() &&
                    ( clip->begin() == c->begin() && clip->end() == c->end() ) ) )
        {
            vlmcWarning() << "Clip already loaded.";
            return false;
        }
    }
    m_clips[clip->uuid()] = clip;
    emit newClipLoaded( clip );
    return true;
}

void
MediaContainer::clear()
{
    QHash<QUuid, Clip*>::iterator  it = m_clips.begin();
    QHash<QUuid, Clip*>::iterator  end = m_clips.end();

    while ( it != end )
    {
        emit clipRemoved( it.key() );
        it.value()->clear();
        it.value()->deleteLater();
        ++it;
    }
    m_clips.clear();
}

void
MediaContainer::removeAll()
{
    QHash<QUuid, Clip*>::iterator  it = m_clips.begin();
    QHash<QUuid, Clip*>::iterator  end = m_clips.end();

    while ( it != end )
    {
        emit clipRemoved( it.key() );
        ++it;
    }
    m_clips.clear();
}

void
MediaContainer::deleteClip( const QUuid &uuid )
{
    QHash<QUuid, Clip*>::iterator  it = m_clips.find( uuid );
    if ( it != m_clips.end() )
    {
        Clip* clip = it.value();
        m_clips.remove( uuid );
        emit clipRemoved( uuid );
        // don't use delete as the clip may be used in the slot that'll handle clipRemoved signal.
        clip->deleteLater();
    }
}

const QHash<QUuid, Clip*>&
MediaContainer::clips() const
{
    return m_clips;
}

void
MediaContainer::reloadAllClips()
{
    for ( auto *c: m_clips )
        emit newClipLoaded( c );
}

Clip*
MediaContainer::getParent()
{
    return m_parent;
}

quint32
MediaContainer::count() const
{
    return m_clips.size();
}
