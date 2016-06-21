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
#include "Main/Core.h"
#include "Backend/VLC/VLCSource.h"
#include "Library/Library.h"
#include "Media/Media.h"
#include "Project/Workspace.h"
#include <QVariant>

const int   Clip::DefaultFPS = 30;

Clip::Clip( Media *media, qint64 begin /*= 0*/, qint64 end /*= -1*/, const QString& uuid /*= QString()*/ ) :
        Workflow::Helper( begin, end, uuid ),
        m_media( media ),
        m_parent( media->baseClip() ),
        m_clipWorkflow( nullptr )
{
    int64_t nbSourceFrames = media->source()->nbFrames();
    if ( end == -1 )
        m_end = nbSourceFrames;
    m_beginPosition = (float)begin / (float)nbSourceFrames;
    m_endPosition = (float)end / (float)nbSourceFrames;
    m_childs = new MediaContainer( this );
    m_rootClip = media->baseClip();
    computeLength();
    connect( media, &Media::metaDataComputed,
             this, &Clip::mediaMetadataUpdated );
}

Clip::Clip( Clip *parent, qint64 begin /*= -1*/, qint64 end /*= -1*/,
            const QString &uuid /*= QString()*/ ) :
        Workflow::Helper( begin, end, uuid ),
        m_media( parent->media() ),
        m_rootClip( parent->rootClip() ),
        m_parent( parent )
{
    int64_t nbSourceFrames = parent->media()->source()->nbFrames();
    if ( begin < 0 )
        m_begin = parent->m_begin;
    if ( end < 0 )
        m_end = parent->m_end;
    m_beginPosition = (float)begin / (float)nbSourceFrames;
    m_endPosition = (float)end / (float)nbSourceFrames;
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
Clip::media()
{
    return m_media;
}

const Media*
Clip::media() const
{
    return m_media;
}

qint64
Clip::lengthSecond() const
{
    return m_lengthSeconds;
}

void
Clip::computeLength()
{
    int64_t sourceLengthSeconds = m_media->source()->length() / 1000;
    m_nbFrames = m_end - m_begin;
    m_lengthSeconds = qRound64( ( m_endPosition - m_beginPosition ) * sourceLengthSeconds );
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
Clip::parent()
{
    return m_parent;
}

const Clip*
Clip::parent() const
{
    return m_parent;
}

MediaContainer*
Clip::mediaContainer()
{
    return m_childs;
}

const MediaContainer*
Clip::mediaContainer() const
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

QVariant
Clip::toVariant() const
{
    QVariantHash h = {
        { "uuid", m_uuid.toString() },
        { "metatags", m_metaTags },
        { "notes", m_notes }
    };
    if ( isRootClip() )
        h.insert( "media", m_media->toVariant() );
    else
    {
        h.insert( "parent", m_parent->uuid().toString() );
        h.insert( "begin", m_begin );
        h.insert( "end", m_end );
    }
    return QVariant( h );

}

QVariant
Clip::toVariantFull() const
{
    QVariantHash h = toVariant().toHash();
    if ( m_childs->count() > 0 )
    {
        QVariantList l;
        for ( const auto& c : m_childs->clips() )
            l << c->toVariant();
        h.insert( "subClips", l );
    }
    return h;
}

Clip::Formats
Clip::formats() const
{
    return m_formats;
}

void
Clip::setFormats( Formats formats )
{
    if ( formats.testFlag( Clip::None ) )
        m_formats = Clip::None;
    m_formats = formats;
}

ClipWorkflow*
Clip::clipWorkflow() const
{
    return m_clipWorkflow;
}

void
Clip::setClipWorkflow( ClipWorkflow *cw )
{
    m_clipWorkflow = cw;
}

void
Clip::mediaMetadataUpdated()
{
    Q_ASSERT ( isRootClip() == true );
    if ( m_end == 0 )
    {
        m_begin = 0;
        m_end = m_media->source()->nbFrames();
        m_beginPosition = 0.0f;
        m_endPosition = 1.0f;
        computeLength();
    }
}
