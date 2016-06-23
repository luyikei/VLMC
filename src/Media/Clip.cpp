/*****************************************************************************
 * Clip.cpp : Represents a basic container for media informations.
 *****************************************************************************
 * Copyright (C) 2008-2016 VideoLAN
 *
 * Authors: Hugo Beauzée-Luyssen <hugo@beauzee.fr>
 *          Yikei Lu    <luyikei.qmltu@gmail.com>
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
#include "Backend/MLT/MLTProducer.h"
#include "Library/Library.h"
#include "Media/Media.h"
#include "Project/Workspace.h"
#include "EffectsEngine/EffectHelper.h"
#include <QVariant>

Clip::Clip( Media *media, qint64 begin /*= 0*/, qint64 end /*= Backend::IProducer::EndOfMedia */, const QString& uuid /*= QString()*/ ) :
        Workflow::Helper( uuid ),
        m_media( media ),
        m_producer( m_media->producer()->cut( begin, end ) ),
        m_parent( media->baseClip() ),
        m_clipWorkflow( nullptr )
{
    m_childs = new MediaContainer( this );
    m_rootClip = media->baseClip();
}

Clip::Clip( Clip *parent, qint64 begin /*= -1*/, qint64 end /*= -2*/,
            const QString &uuid /*= QString()*/ ) :
        Workflow::Helper( uuid ),
        m_media( parent->media() ),
        m_rootClip( parent->rootClip() ),
        m_parent( parent )
{
    m_childs = new MediaContainer( this );
    if ( begin == -1 )
        begin = parent->begin();
    else
        begin = parent->begin() + begin;

    if ( end == Backend::IProducer::EndOfParent )
        end = parent->end();
    else
        end = parent->begin() + end;
    m_producer = parent->producer()->cut( begin, end );
}

Clip::~Clip()
{
    emit unloaded( this );
    delete m_childs;
    delete m_producer;
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
    return qRound64( m_producer->playableLength() / m_producer->fps() );
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

qint64
Clip::begin() const
{
    return m_producer->begin();
}

qint64
Clip::end() const
{
    return m_producer->end();
}

void
Clip::setBegin( qint64 begin )
{
    m_producer->setBegin( begin );
}

void
Clip::setEnd( qint64 end )
{
    m_producer->setEnd( end );
}

qint64
Clip::length() const
{
    return m_producer->playableLength();
}

void
Clip::setBoundaries( qint64 begin, qint64 end )
{
    m_producer->setBoundaries( begin, end );
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
        h.insert( "begin", begin() );
        h.insert( "end", end() );
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

Backend::IProducer*
Clip::producer()
{
    return m_producer;
}

void
Clip::mediaMetadataUpdated()
{
}

