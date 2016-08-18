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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "Clip.h"
#include "Main/Core.h"
#include "Backend/MLT/MLTInput.h"
#include "Library/Library.h"
#include "Media/Media.h"
#include "Project/Workspace.h"
#include "EffectsEngine/EffectHelper.h"
#include "Tools/VlmcDebug.h"
#include <QVariant>

Clip::Clip( QSharedPointer<Media> media, qint64 begin /*= 0*/, qint64 end /*= Backend::IInput::EndOfMedia */, const QString& uuid /*= QString()*/ ) :
        Workflow::Helper( uuid ),
        m_media( media ),
        m_input( std::move( m_media->input()->cut( begin, end ) ) ),
        m_parent( media->baseClip() ),
        m_isLinked( false )
{
    m_rootClip = media->baseClip();
    Formats f;
    if ( media->input()->hasAudio() == true )
        f |= Clip::Audio;
    if ( media->input()->hasVideo() == true )
        f |= Clip::Video;
    setFormats( f );
}

Clip::Clip( Clip *parent, qint64 begin /*= -1*/, qint64 end /*= -2*/,
            const QString &uuid /*= QString()*/ ) :
        Workflow::Helper( uuid ),
        m_media( parent->media() ),
        m_rootClip( parent->rootClip() ),
        m_parent( parent )
{
    if ( begin == -1 )
        begin = parent->begin();
    else
        begin = parent->begin() + begin;

    if ( end == Backend::IInput::EndOfParent )
        end = parent->end();
    else
        end = parent->begin() + end;
    m_input = parent->input()->cut( begin, end );
    setFormats( parent->formats() );
}

Clip::~Clip()
{
    emit unloaded( this );
}

QSharedPointer<Media>
Clip::media()
{
    return m_media;
}

QSharedPointer<const Media>
Clip::media() const
{
    return m_media;
}

qint64
Clip::lengthSecond() const
{
    return qRound64( m_input->playableLength() / m_input->fps() );
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
    return m_input->begin();
}

qint64
Clip::end() const
{
    return m_input->end();
}

void
Clip::setBegin( qint64 begin )
{
    m_input->setBegin( begin );
}

void
Clip::setEnd( qint64 end )
{
    m_input->setEnd( end );
}

qint64
Clip::length() const
{
    return m_input->playableLength();
}

void
Clip::setBoundaries( qint64 begin, qint64 end )
{
    m_input->setBoundaries( begin, end );
}

void
Clip::setLinkedClipUuid( const QUuid& uuid )
{
    m_linkedClipUuid = uuid;
}

const QUuid&
Clip::linkedClipUuid() const
{
    return m_linkedClipUuid;
}

void
Clip::setLinked( bool isLinked )
{
    m_isLinked = isLinked;
}

bool
Clip::isLinked() const
{
    return m_isLinked;
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

bool
Clip::addSubclip( Clip *clip )
{
    if ( m_subclips.contains( clip->uuid() ) == true )
        return false;
    m_subclips[clip->uuid()] = clip;
    emit subclipAdded( clip );
}

void
Clip::clear()
{
    m_subclips.clear();
}

QVariant
Clip::toVariant() const
{
    QVariantHash h = {
        { "uuid", m_uuid.toString() },
        { "metatags", m_metaTags },
        { "notes", m_notes },
        { "formats", (int)formats() }
    };
    if ( isRootClip() )
        h.insert( "media", m_media->toVariant() );
    else
    {
        h.insert( "parent", m_parent->uuid().toString() );
        h.insert( "begin", begin() );
        h.insert( "end", end() );
    }
    if ( isLinked() == true )
    {
        h.insert( "linkedClip", m_linkedClipUuid );
        h.insert( "linked", true );
    }
    else
        h.insert( "linked", false );
    h.insert( "filters", EffectHelper::toVariant( m_input.get() ) );
    return QVariant( h );

}

QVariant
Clip::toVariantFull() const
{
    QVariantHash h = toVariant().toHash();
    if ( m_subclips.isEmpty() == true )
        return h;
    QVariantList l;
    for ( const auto& c : m_subclips.values() )
        l << c->toVariant();
    h.insert( "subClips", l );
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

Backend::IInput*
Clip::input()
{
    return m_input.get();
}

Clip*
Clip::fromVariant( const QVariant& v )
{
    auto m = v.toMap();

    if ( m.contains( "parent" ) )
    {
        vlmcWarning() << "Refusing to load a root clip with a parent field";
        return nullptr;
    }

    auto mediaId = m["media"].toLongLong();
    if ( mediaId == 0 )
    {
        vlmcWarning() << "Refusing to load an invalid root clip with no base media";
        return nullptr;
    }

    auto uuid = m["uuid"].toString();
    if ( uuid.isEmpty() == true )
    {
        vlmcWarning() << "Refusing to load an invalid root clip with no UUID";
        return nullptr;
    }

    auto media = Core::instance()->library()->media( mediaId );
    auto clip = new Clip( media, 0, -1, uuid );

    clip->loadVariant( m );

    return clip;
}

Clip*
Clip::fromVariant( const QVariant& v, Clip* parent )
{
    auto m = v.toMap();

    if ( m.contains( "parent" ) == false )
    {
        vlmcWarning() << "Refusing to load a subclip with no parent field";
        return nullptr;
    }

    auto mediaMrl = m["media"].toString();
    if ( mediaMrl.isEmpty() == true )
    {
        vlmcWarning() << "Refusing to load an invalid root clip with no base media";
        return nullptr;
    }

    auto uuid = m["uuid"].toString();
    if ( uuid.isEmpty() == true )
    {
        vlmcWarning() << "Refusing to load an invalid root clip with no UUID";
        return nullptr;
    }
    auto begin = m["begin"].toLongLong();
    auto end = m["end"].toLongLong();

    auto clip = new Clip( parent, begin, end, uuid );
    clip->loadVariant( m );
    return clip;
}

void
Clip::loadVariant( const QVariantMap& m )
{
    if ( m.contains( "subClips" ) )
    {
        auto children = m["subClips"].toList();
        for ( const auto& clipMap : children )
            addSubclip( fromVariant( clipMap, this ) );
    }
    if ( m.contains( "filters" ) )
    {
        const auto& filters = m["filters"].toList();
        for ( const auto& f : filters )
            EffectHelper::loadFromVariant( f, input() );
    }
}

void
Clip::mediaMetadataUpdated()
{
}

