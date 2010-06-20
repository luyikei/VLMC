/*****************************************************************************
 * Clip.cpp : Represents a basic container for media informations.
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
  * This file contains the Clip class implementation.
  */

#include <QtDebug>

#include "Clip.h"
#include "Library.h"
#include "Media.h"
#include "Workspace.h"

const int   Clip::DefaultFPS = 30;

Clip::Clip( Media *media, qint64 begin /*= 0*/, qint64 end /*= -1*/, const QString& uuid /*= QString()*/ ) :
        m_media( media ),
        m_begin( begin ),
        m_end( end ),
        m_parent( media->baseClip() )
{
    if ( end == -1 )
        m_end = media->nbFrames();
    if ( uuid.isEmpty() == true )
        m_uuid = QUuid::createUuid();
    else
        m_uuid = QUuid( uuid );
    m_childs = new MediaContainer( this );
    m_rootClip = media->baseClip();
    computeLength();
    connect( media, SIGNAL( metaDataComputed( const Media* ) ),
             this, SLOT( mediaMetadataUpdated( const Media* ) ) );
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
Clip::length() const
{
    return m_length;
}

qint64
Clip::lengthSecond() const
{
    return m_lengthSeconds;
}

void
Clip::computeLength()
{
    if ( m_media->inputType() == Media::File )
    {
        float   fps = m_media->fps();
        if ( fps < 0.1f )
            fps = Clip::DefaultFPS;
        m_length = m_end - m_begin;
        m_lengthSeconds = qRound64( (float)m_length / fps );
    }
    else
    {
        m_length = 0;
        m_lengthSeconds = 0;
    }
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
    if ( m_rootClip == NULL )
        return this;
    return m_rootClip;
}

bool
Clip::isRootClip() const
{
    return ( m_rootClip == NULL );
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

void
Clip::save( QXmlStreamWriter &project )
{
    project.writeStartElement( "clip" );
    if ( isRootClip() == true )
    {
        if ( m_media->isInWorkspace() == true )
            project.writeAttribute( "media", Workspace::pathInProjectDir( m_media ) );
        else
            project.writeAttribute( "media", m_media->fileInfo()->absoluteFilePath() );
    }
    else
    {
        project.writeAttribute( "parent", m_parent->uuid() );
        project.writeAttribute( "begin", QString::number( m_begin ) );
        project.writeAttribute( "end", QString::number( m_end ) );
    }
    project.writeAttribute( "uuid", m_uuid.toString() );
    project.writeAttribute( "metatags", m_metaTags.join( "," ) );
    project.writeAttribute( "notes", m_notes );
    if ( m_childs->count() > 0 )
    {
        project.writeStartElement( "subClips" );
        m_childs->save( project );
        project.writeEndElement();
    }
    project.writeEndElement();
}

QString
Clip::fullId() const
{
    const Clip* c = this;
    QString     id = m_uuid.toString();
    while ( c->isRootClip() == false )
    {
        c = c->getParent();
        id = c->uuid() + '/' + id;
    }
    return id;
}

bool
Clip::isChild( const QUuid &uuid) const
{
    Clip*   c = m_parent;

    while ( c != NULL )
    {
        if ( c->m_uuid == uuid )
            return true;
        c = c->m_parent;
    }
    return false;
}

void
Clip::mediaMetadataUpdated( const Media *media )
{
    Q_ASSERT ( isRootClip() == true && m_media == media );
    if ( m_end == 0 )
    {
        m_begin = 0;
        m_end = m_media->nbFrames();
        computeLength();
    }
}
