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
#include <QDomElement>
#include <QUuid>

#include "Clip.h"
#include "MediaContainer.h"
#include "Media.h"
#include "MetaDataManager.h"

#include <QtDebug>

MediaContainer::MediaContainer( Clip* parent /*= NULL*/ ) : m_parent( parent )
{
}

Clip*
MediaContainer::clip( const QUuid& uuid )
{
    QHash<QUuid, Clip*>::iterator   it = m_clips.find( uuid );
    if ( it != m_clips.end() )
        return it.value();
    return NULL;
}

Clip*
MediaContainer::clip( const QString &uuid )
{
    MediaContainer      *mc = this;
    Clip                *clip;
    QStringList         ids = uuid.split( '/' );

    foreach ( QString id, ids )
    {
        clip = mc->clip( QUuid( id ) );
        if ( clip == NULL )
            return NULL;
        mc = clip->getChilds();
    }
    return clip;
}

void
MediaContainer::addMedia( Media *media )
{
    m_clips[media->baseClip()->uuid()] = media->baseClip();
    emit newClipLoaded( media->baseClip() );
}

Media*
MediaContainer::addMedia( const QFileInfo& fileInfo, const QString& uuid )
{
    if ( QFile::exists( fileInfo.absoluteFilePath() ) == false )
        return NULL;
    foreach( Clip* it, m_clips.values() )
    {
        if ( it->getMedia()->fileInfo()->filePath() == fileInfo.filePath() )
            return NULL;
    }
    Media* media = new Media( fileInfo.filePath(), uuid );
    MetaDataManager::getInstance()->computeMediaMetadata( media );
    return media;
}

bool
MediaContainer::mediaAlreadyLoaded( const QFileInfo& fileInfo )
{
    foreach( Clip* clip, m_clips.values() )
    {
        if ( clip->getMedia()->fileInfo()->filePath() == fileInfo.filePath() )
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
             ( clip->getMedia()->fileInfo() == c->getMedia()->fileInfo() &&
                    ( clip->begin() == c->begin() && clip->end() == c->end() ) ) )
        {
            qDebug() << "Clip already loaded.";
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

Clip*
MediaContainer::removeClip( const QUuid &uuid )
{
    QHash<QUuid, Clip*>::iterator  it = m_clips.find( uuid );
    if ( it != m_clips.end() )
    {
        Clip* clip = it.value();
        m_clips.remove( uuid );
        emit clipRemoved( uuid );
        return clip;
    }
    return NULL;
}

Clip*
MediaContainer::removeClip( const Clip* clip )
{
    return removeClip( clip->uuid() );
}

const QHash<QUuid, Clip*>&
MediaContainer::clips() const
{
    return m_clips;
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

void
MediaContainer::save( QXmlStreamWriter &project )
{
    foreach ( Clip* c, m_clips.values() )
        c->save( project );
}

void
MediaContainer::load( const QDomElement &clips, MediaContainer *parentMC )
{
    QDomElement clip = clips.firstChild().toElement();

    while ( clip.isNull() == false )
    {
        QString     uuid = clip.attribute( "uuid" );
        QString     metatags = clip.attribute( "metatags" );
        QString     notes = clip.attribute( "notes" );
        Clip        *c;

        if ( clip.hasAttribute( "media" ) == true )
        {
            QString media = clip.attribute( "media" );
            Media*  m = m_medias[media];
            if ( m != NULL )
            {
                c = new Clip( m, 0, -1, uuid );
                m->setBaseClip( c );
                addClip( c );
            }
        }
        else
        {
            QString     parent = clip.attribute( "parent" );
            QString     begin = clip.attribute( "begin" );
            QString     end = clip.attribute( "end" );

            if ( parent.isEmpty() == false )
            {
                Clip*   p = parentMC->clip( QUuid( parent ) );
                c = new Clip( p, begin.toLongLong(), end.toLongLong(), uuid );
                addClip( c );
            }
        }
        if ( metatags.isEmpty() == false )
            c->setMetaTags( metatags.split( ',' ) );
        c->setNotes( notes );
        QDomElement subClips = clip.firstChildElement( "subClips" );
        if ( subClips.isNull() == false )
            c->getChilds()->load( subClips, this );
        clip = clip.nextSibling().toElement();
    }
}
