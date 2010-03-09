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

Clip::Clip( Media *parent, const QString& uuid ) :
        m_parent( parent ),
        m_begin( 0 ),
        m_end( parent->nbFrames() ),
        m_maxBegin( 0 ),
        m_maxEnd( parent->nbFrames() ),
        m_rootClip( NULL )
{
    Q_ASSERT( parent->baseClip() == NULL );
    if ( uuid.isEmpty() == true )
        m_uuid = QUuid::createUuid();
    else
        m_uuid = QUuid( uuid );
    computeLength();
}

Clip::Clip( Clip *clip, qint64 begin /*= 0*/, qint64 end /*= -1*/ ) :
        m_parent( clip->m_parent ),
        m_begin( begin ),
        m_end( end ),
        m_metaTags( clip->m_metaTags ),
        m_notes( clip->m_notes ),
        m_maxBegin( clip->m_begin ),
        m_maxEnd( clip->m_end ),
        m_rootClip( clip->getParent()->baseClip() )
{
    if ( begin == -1 )
        m_begin = clip->begin();
    if ( end == -1 )
        m_end = clip->end();
    m_uuid = QUuid::createUuid();
    computeLength();
}

Clip::Clip( Media *parent, qint64 begin, qint64 end /*= -1*/,
            const QString &uuid /*= QString()*/ ) :
        m_parent( parent ),
        m_begin( begin ),
        m_end( end ),
        m_maxBegin( begin ),
        m_maxEnd( end ),
        m_rootClip( parent->baseClip() )
{
    if ( end < 0 )
    {
        m_end = parent->nbFrames();
        m_maxEnd = m_end;
    }
    if ( uuid.isEmpty() == true )
        m_uuid = QUuid::createUuid();
    else
        m_uuid = QUuid( uuid );
    computeLength();
}

Clip::~Clip()
{
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
Clip::getParent()
{
    return m_parent;
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
    if ( m_parent->inputType() == Media::File )
    {
        float   fps = m_parent->fps();
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
    return m_rootClip;
}

bool
Clip::isRootClip() const
{
    return ( m_rootClip == NULL );
}
