/*****************************************************************************
 * Clip.cpp : Represents a basic container for media informations.
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

/** \file
  * This file contains the Clip class implementation.
  */

#include "Clip.h"
#include "Backend/ISource.h"
#include "Library/Library.h"
#include "Media/Media.h"
#include "Project/Workspace.h"

const int   Clip::DefaultFPS = 30;

Clip::Clip( Media *media, qint64 begin /*= 0*/, qint64 end /*= -1*/, const QString& uuid /*= QString()*/ ) :
        m_media( media ),
        m_begin( begin ),
        m_end( end ),
        m_parent( media->baseClip() )
{
    if ( end == -1 )
        m_end = media->source()->nbFrames();
    if ( uuid.isEmpty() == true )
        m_uuid = QUuid::createUuid();
    else
        m_uuid = QUuid( uuid );
    m_childs = new MediaContainer( this );
    m_rootClip = media->baseClip();
    computeLength();
    connect( media, &Media::metaDataComputed,
             this, &Clip::mediaMetadataUpdated );
}

Clip::Clip( Clip *parent, qint64 begin /*= -1*/, qint64 end /*= -1*/,
            const QString &uuid /*= QString()*/ ) :
        m_media( parent->getMedia() ),
        m_begin( begin ),
        m_end( end ),
        m_rootClip( parent->rootClip() ),
        m_parent( parent )
{
    if ( begin < 0 )
        m_begin = parent->m_begin;
    if ( end < 0 )
        m_end = parent->m_end;
    if ( uuid.isEmpty() == true )
        m_uuid = QUuid::createUuid();
    else
        m_uuid = QUuid( uuid );
    m_childs = new MediaContainer( this );
    computeLength();
}

Clip::~Clip()
{
    emit unloaded( this );
    delete m_childs;
    if ( isRootClip() == true )
        delete m_media;
}

Media*
Clip::getMedia()
{
    return m_media;
}

const Media*
Clip::getMedia() const
{
    return m_media;
}

qint64
Clip::nbFrames() const
{
    return m_nbFrames;
}

qint64
Clip::lengthSecond() const
{
    return m_lengthSeconds;
}

void
Clip::computeLength()
{
    float   fps = m_media->source()->fps();
    if ( fps < 0.1f )
        fps = Clip::DefaultFPS;
    m_nbFrames = m_end - m_begin;
    m_lengthSeconds = qRound64( (float)m_nbFrames / fps );
}

const QStringList&
Clip::metaTags() const
{
    return m_metaTags;
}

void
Clip::setMetaTags( const QStringList &tags )
{
    m_metaTags = tags;
}

bool
Clip::matchMetaTag( const QString &tag ) const
{
    if ( tag.length() == 0 )
        return true;
    if ( m_parent && m_parent->matchMetaTag( tag ) == true )
        return true;
    QString metaTag;
    foreach ( metaTag, m_metaTags )
    {
        if ( metaTag.startsWith( tag, Qt::CaseInsensitive ) == true )
            return true;
    }
    return false;
}

const QString&
Clip::notes() const
{
    return m_notes;
}

void
Clip::setNotes( const QString &notes )
{
    m_notes = notes;
}

const QUuid&
Clip::uuid() const
{
    Q_ASSERT( m_uuid.isNull() == false );
    return m_uuid;
}

void
Clip::setUuid( const QUuid &uuid )
{
    m_uuid = uuid;
}

Clip*
Clip::rootClip()
{
    if ( m_rootClip == nullptr )
        return this;
    return m_rootClip;
}

bool
Clip::isRootClip() const
{
    return ( m_rootClip == nullptr );
}

Clip*
Clip::getParent()
{
    return m_parent;
}

const Clip*
Clip::getParent() const
{
    return m_parent;
}

MediaContainer*
Clip::getChilds()
{
    return m_childs;
}

const MediaContainer*
Clip::getChilds() const
{
    return m_childs;
}

bool
Clip::addSubclip( Clip *clip )
{
    return m_childs->addClip( clip );
}

void
Clip::clear()
{
    m_childs->clear();
}

QString
Clip::fullId() const
{
    const Clip* c = this;
    QString     id = m_uuid.toString();
    while ( c->isRootClip() == false )
    {
        c = c->getParent();
        id = c->uuid().toString() + '/' + id;
    }
    return id;
}

bool
Clip::isChild( const QUuid &uuid) const
{
    Clip*   c = m_parent;

    while ( c != nullptr )
    {
        if ( c->m_uuid == uuid )
            return true;
        c = c->m_parent;
    }
    return false;
}

void
Clip::mediaMetadataUpdated()
{
    Q_ASSERT ( isRootClip() == true );
    if ( m_end == 0 )
    {
        m_begin = 0;
        m_end = m_media->source()->nbFrames();
        computeLength();
    }
}
