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

Clip::Clip( QSharedPointer<Media> media, qint64 begin /*= 0*/, qint64 end /*= Backend::IInput::EndOfMedia */, const QUuid& uuid /*= QString()*/ ) :
        Workflow::Helper( uuid ),
        m_media( media ),
        m_input( media->input()->cut( begin, end ) ),
        m_onTimeline( false )
{
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

bool
Clip::onTimeline() const
{
    return m_onTimeline;
}

void
Clip::setOnTimeline( bool onTimeline )
{
    if ( m_onTimeline != onTimeline )
        emit onTimelineChanged( onTimeline );
    m_onTimeline = onTimeline;
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

QVariant
Clip::toVariant() const
{
    QVariantHash h = {
        { "libraryUuid", m_uuid.toString() },
        { "metatags", m_metaTags },
        { "notes", m_notes },
    };
    h.insert( "begin", begin() );
    h.insert( "end", end() );
    return QVariant( h );

}

Backend::IInput*
Clip::input()
{
    return m_input.get();
}
