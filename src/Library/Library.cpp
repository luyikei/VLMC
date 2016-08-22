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
#include "MediaLibraryModel.h"
#include "Project/Project.h"
#include "Settings/Settings.h"
#include "Tools/VlmcDebug.h"

#include <QVariant>
#include <QHash>
#include <QUuid>

Library::Library( Settings* vlmcSettings, Settings *projectSettings )
    : m_initialized( false )
    , m_cleanState( true )
    , m_settings( new Settings )
{
    // Setting up the external media library
    m_ml.reset( NewMediaLibrary() );
    m_ml->setVerbosity( medialibrary::LogLevel::Warning );
    m_videoModel = new MediaLibraryModel( *m_ml, medialibrary::IMedia::Type::VideoType, this );
    m_audioModel = new MediaLibraryModel( *m_ml, medialibrary::IMedia::Type::AudioType, this );

    auto s = vlmcSettings->createVar( SettingValue::List, QStringLiteral( "vlmc/mlDirs" ), QVariantList(),
                        "Media Library folders", "List of folders VLMC will search for media files",
                         SettingValue::Folders );
    connect( s, &SettingValue::changed, this, &Library::mlDirsChanged );
    auto ws = vlmcSettings->value( "vlmc/WorkspaceLocation" );
    connect( ws, &SettingValue::changed, this, &Library::workspaceChanged );

    // Setting up the project section of the Library
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

medialibrary::MediaPtr
Library::mlMedia( qint64 mediaId )
{
    return m_ml->media( mediaId );
}

MediaLibraryModel*
Library::model(Library::MediaType type) const
{
    switch ( type )
    {
        case MediaType::Video:
            return m_videoModel;
        case MediaType::Audio:
            return m_audioModel;
    }
    Q_UNREACHABLE();
}

QSharedPointer<Clip>
Library::clip( const QUuid& uuid )
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

void
Library::mlDirsChanged(const QVariant& value)
{
    // We can't handle this event without an initialized media library, and therefor without a valid
    // workspace. In theory, the workspace SettingValue is created before the mlDirs,
    // so it should be loaded before.
    Q_ASSERT( m_initialized == true );

    const auto list = value.toStringList();
    Q_ASSERT( list.empty() == false );
    for ( const auto f : list )
        m_ml->discover( f.toStdString() );
}

void
Library::workspaceChanged(const QVariant& workspace)
{
    Q_ASSERT( workspace.isNull() == false && workspace.canConvert<QString>() );

    if ( m_initialized == false )
    {
        auto w = workspace.toString().toStdString();
        Q_ASSERT( w.empty() == false );
        // Initializing the medialibrary doesn't start new folders discovery.
        // This will happen after the first call to IMediaLibrary::discover()
        m_ml->initialize( w + "/ml.db", w + "/thumbnails/", this );
        m_initialized = true;
    }
    //else FIXME, and relocate the media library
}

void
Library::onMediaAdded( std::vector<medialibrary::MediaPtr> mediaList )
{
    for ( auto m : mediaList )
    {
        switch ( m->type() )
        {
        case medialibrary::IMedia::Type::VideoType:
            m_videoModel->addMedia( m );
            break;
        case medialibrary::IMedia::Type::AudioType:
            m_audioModel->addMedia( m );
            break;
        default:
            Q_UNREACHABLE();
        }
    }
}

void
Library::onMediaUpdated( std::vector<medialibrary::MediaPtr> mediaList )
{
    for ( auto m : mediaList )
    {
        switch ( m->type() )
        {
        case medialibrary::IMedia::Type::VideoType:
            m_videoModel->updateMedia( m );
            break;
        case medialibrary::IMedia::Type::AudioType:
            m_audioModel->updateMedia( m );
            break;
        default:
            Q_UNREACHABLE();
        }
    }
}

void
Library::onMediaDeleted( std::vector<int64_t> mediaList )
{
    for ( auto id : mediaList )
    {
        // We can't know the media type, however ID are unique regardless of the type
        // so we are sure that we will remove the correct media.
        if ( m_videoModel->removeMedia( id ) == true )
            continue;
        m_audioModel->removeMedia( id );
    }
}

void
Library::onArtistsAdded( std::vector<medialibrary::ArtistPtr> )
{
}

void
Library::onArtistsModified( std::vector<medialibrary::ArtistPtr> )
{
}

void
Library::onArtistsDeleted( std::vector<int64_t> )
{
}

void
Library::onAlbumsAdded( std::vector<medialibrary::AlbumPtr> )
{
}

void
Library::onAlbumsModified( std::vector<medialibrary::AlbumPtr> )
{
}

void
Library::onAlbumsDeleted( std::vector<int64_t> )
{
}

void
Library::onTracksAdded( std::vector<medialibrary::AlbumTrackPtr> )
{
}

void
Library::onTracksDeleted( std::vector<int64_t> )
{
}

void
Library::onDiscoveryStarted( const std::string& entryPoint )
{
    emit discoveryStarted( QString::fromStdString( entryPoint ) );
}

void
Library::onDiscoveryCompleted( const std::string& entryPoint )
{
    if ( entryPoint.empty() == true )
    {
        m_videoModel->refresh();
        m_audioModel->refresh();
    }
    emit discoveryCompleted( QString::fromStdString( entryPoint ) );
}

void
Library::onParsingStatsUpdated( uint32_t percent )
{
    emit progressUpdated( static_cast<int>( percent ) );
}
