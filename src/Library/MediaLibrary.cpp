/*****************************************************************************
 * MediaLibrary.cpp: Wraps the media library
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

#include "MediaLibrary.h"
#include "Settings/Settings.h"
#include "MediaLibraryModel.h"

MediaLibrary::MediaLibrary( Settings* settings )
    : m_videoModel( nullptr )
    , m_initialized( false )
{
    m_ml.reset( NewMediaLibrary() );
    auto s = settings->createVar( SettingValue::List, QStringLiteral( "vlmc/mlDirs" ), QVariantList(),
                        "Media Library folders", "List of folders VLMC will search for media files",
                         SettingValue::Folders );
    connect( s, &SettingValue::changed, this, &MediaLibrary::mlDirsChanged );
    auto ws = settings->value( "vlmc/WorkspaceLocation" );
    connect( ws, &SettingValue::changed, this, &MediaLibrary::workspaceChanged );
}

MediaLibraryModel* MediaLibrary::model( MediaLibrary::MediaType type ) const
{
    switch ( type )
    {
        case MediaType::Video:
            return m_videoModel;
        case MediaType::Audio:
            return nullptr;
    }
    Q_UNREACHABLE();
}

void
MediaLibrary::mlDirsChanged( const QVariant& value )
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
MediaLibrary::workspaceChanged( const QVariant& workspace )
{
    Q_ASSERT( workspace.isNull() == false && workspace.canConvert<QString>() );

    if ( m_initialized == false )
    {
        auto w = workspace.toString().toStdString();
        Q_ASSERT( w.empty() == false );
        m_ml->initialize( w + "/ml.db", w + "/thumbnails/", this );
        //FIXME: Race condition, this could trigger onMediaAdded before m_videoModel has been created.
        //However, we need the model to be created AFTER the ML gets initialized
        m_videoModel = new MediaLibraryModel( *m_ml, medialibrary::IMedia::Type::VideoType, this );
        m_initialized = true;
    }
    //else FIXME, and relocate the media library
}

void MediaLibrary::onMediaAdded( std::vector<medialibrary::MediaPtr> mediaList )
{
    for ( auto m : mediaList )
        m_videoModel->addMedia( m );
}

void MediaLibrary::onMediaUpdated( std::vector<medialibrary::MediaPtr> )
{
}

void MediaLibrary::onMediaDeleted( std::vector<int64_t> )
{
}

void MediaLibrary::onArtistsAdded( std::vector<medialibrary::ArtistPtr> )
{
}

void MediaLibrary::onArtistsModified( std::vector<medialibrary::ArtistPtr> )
{
}

void MediaLibrary::onArtistsDeleted( std::vector<int64_t> )
{
}

void MediaLibrary::onAlbumsAdded( std::vector<medialibrary::AlbumPtr> )
{
}

void MediaLibrary::onAlbumsModified( std::vector<medialibrary::AlbumPtr> )
{
}

void MediaLibrary::onAlbumsDeleted( std::vector<int64_t> )
{
}

void MediaLibrary::onTracksAdded( std::vector<medialibrary::AlbumTrackPtr> )
{
}

void MediaLibrary::onTracksDeleted( std::vector<int64_t> )
{
}

void MediaLibrary::onDiscoveryStarted( const std::string& entryPoint )
{
    emit discoveryStarted( QString::fromStdString( entryPoint ) );
}

void MediaLibrary::onDiscoveryCompleted( const std::string& entryPoint )
{
    emit discoveryCompleted( QString::fromStdString( entryPoint ) );
}

void MediaLibrary::onReloadStarted( const std::string& entryPoint )
{
    emit reloadStarted( QString::fromStdString( entryPoint ) );
}

void MediaLibrary::onReloadCompleted( const std::string& entryPoint )
{
    emit reloadCompleted( QString::fromStdString( entryPoint ) );
}

void MediaLibrary::onParsingStatsUpdated( uint32_t percent )
{
    emit progressUpdated( static_cast<int>( percent ) );
}
