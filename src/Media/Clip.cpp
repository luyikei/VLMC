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
#include "Library.h"

#include "Clip.h"

const int   Clip::DefaultFPS = 30;

Clip::Clip( Media *media, qint64 begin /*= 0*/, qint64 end /*= -1*/, const QString& uuid /*= QString()*/ ) :
        m_media( media ),
        m_begin( begin ),
        m_end( end ),
        m_maxBegin( begin ),
        m_parent( media->baseClip() )
{
    if ( end == -1 )
    {
        m_end = media->nbFrames();
        m_maxEnd = m_end;
    }
    if ( uuid.isEmpty() == true )
        m_uuid = QUuid::createUuid();
    else
        m_uuid = QUuid( uuid );
    m_childs = new MediaContainer( this );
    m_rootClip = media->baseClip();
    computeLength();
}

Clip::Clip( Clip *parent, qint64 begin /*= -1*/, qint64 end /*= -1*/,
            const QString &uuid /*= QString()*/ ) :
        m_parent( parent ),
        m_begin( begin ),
        m_end( end ),
        m_maxBegin( begin ),
        m_maxEnd( end ),
        m_rootClip( parent->rootClip() ),
        m_media( parent->getMedia() )
{
    if ( begin < 0 )
    {
        m_begin = parent->begin();
        m_maxBegin = m_begin;
    }
    if ( end < 0 )
    {
        m_end = parent->getMedia()->nbFrames();
        m_maxEnd = m_end;
    }
    if ( uuid.isEmpty() == true )
        m_uuid = QUuid::createUuid();
    else
        m_uuid = QUuid( uuid );
    m_childs = new MediaContainer( this );
    computeLength();
}

Clip::~Clip()
{
    delete m_childs;
}

qint64
Clip::begin() const
{
    return m_begin;
}

qint64
Clip::end() const
{
    return m_end;
}

Media*
Clip::getMedia()
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
        emit lengthUpdated();
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
    if ( m_parent->matchMetaTag( tag ) == true )
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

void
Clip::setBegin( qint64 begin, bool updateMax /*= false*/ )
{
    Q_ASSERT( begin >= .0f );
    if ( begin == m_begin ) return;
    m_begin = begin;
    if ( updateMax == true )
        m_maxBegin = begin;
    computeLength();
}

void
Clip::setEnd( qint64 end, bool updateMax /*= false*/ )
{
    if ( end == m_end ) return;
    m_end = end;
    if ( updateMax == true )
        m_maxEnd = end;
    computeLength();
}

void
Clip::setBoundaries( qint64 newBegin, qint64 newEnd, bool updateMax /*= false*/ )
{
    Q_ASSERT( newBegin < newEnd );
    if ( newBegin == m_begin && m_end == newEnd )
        return ;
    m_begin = newBegin;
    m_end = newEnd;
    if ( updateMax == true )
    {
        m_maxBegin = newBegin;
        m_maxEnd = newEnd;
    }
    computeLength();
}

qint64
Clip::maxBegin() const
{
    return m_maxBegin;
}

qint64
Clip::maxEnd() const
{
    return m_maxEnd;
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

void
Clip::addSubclip( Clip *clip )
{
    m_childs->addClip( clip );
}

void
Clip::clear()
{
    m_childs->clear();
}
