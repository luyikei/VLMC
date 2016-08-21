/*****************************************************************************
 * Library.cpp: Multimedia library
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
  * This file the library contains class implementation.
  * It's the backend part of the Library widget of vlmc.
  * It can load and unload Medias (Medias.h/Media.cpp)
  * It can load and unload Clips (Clip.h/Clip.cpp)
  */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "Library.h"
#include "Media/Clip.h"
#include "Media/Media.h"
#include "Project/Project.h"
#include "Settings/Settings.h"
#include "Tools/VlmcDebug.h"

#include <QVariant>
#include <QHash>
#include <QUuid>

Library::Library( Settings *projectSettings )
    : m_cleanState( true )
    , m_settings( new Settings )
{
    m_settings->createVar( SettingValue::List, QString( "medias" ), QVariantList(), "", "", SettingValue::Nothing );
    connect( m_settings, &Settings::postLoad, this, &Library::postLoad, Qt::DirectConnection );
    connect( m_settings, &Settings::preSave, this, &Library::preSave, Qt::DirectConnection );

    projectSettings->addSettings( "Library", *m_settings );
}

void
Library::preSave()
{
    QVariantList l;
    for ( auto val : m_media )
        l << val->toVariant();
    m_settings->value( "medias" )->set( l );
    setCleanState( true );
}

void
Library::postLoad()
{
    for ( const auto& var : m_settings->value( "medias" )->get().toList() )
    {
        auto m = Media::fromVariant( var );
        addMedia( m );
    }
}

Library::~Library()
{
    delete m_settings;
}

void
Library::addMedia( QSharedPointer<Media> media )
{
    setCleanState( false );
    if ( m_media.contains( media->id() ) )
        return;
    m_media[media->id()] = media;
    m_clips[media->baseClip()->uuid()] = media->baseClip();
    connect( media.data(), &Media::subclipAdded, [this]( QSharedPointer<Clip> c ) {
        m_clips[c->uuid()] = c;
        setCleanState( false );
    });
    connect( media.data(), &Media::subclipRemoved, [this]( const QUuid& uuid ) {
        m_clips.remove( uuid );
        // This seems wrong, for instance if we undo a clip splitting
        setCleanState( false );
    } );
}

bool
Library::isInCleanState() const
{
    return m_cleanState;
}

QSharedPointer<Media>
Library::media( qint64 mediaId )
{
    return m_media.value( mediaId );
}

QSharedPointer<Clip> Library::clip( const QUuid& uuid )
{
    return m_clips.value( uuid );

}

void
Library::clear()
{
    m_media.clear();
    m_clips.clear();
    setCleanState( true );
}

void
Library::setCleanState( bool newState )
{
    if ( newState != m_cleanState )
    {
        m_cleanState = newState;
        emit cleanStateChanged( newState );
    }
}
