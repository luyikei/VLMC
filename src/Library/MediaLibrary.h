/*****************************************************************************
 * MediaLibrary.h: Wraps the media library
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

#ifndef MEDIALIBRARY_H
#define MEDIALIBRARY_H

#include <medialibrary/IMediaLibrary.h>

#include <QObject>

#include <memory>

class Settings;

class MediaLibrary : public QObject, private medialibrary::IMediaLibraryCb
{
    Q_OBJECT
    Q_DISABLE_COPY( MediaLibrary )

public:
    explicit MediaLibrary( Settings* settings );

private:
    void postLoad();

private:
    virtual void onMediaAdded( std::vector<medialibrary::MediaPtr> media ) override;
    virtual void onMediaUpdated( std::vector<medialibrary::MediaPtr> media ) override;
    virtual void onMediaDeleted( std::vector<int64_t> ids ) override;
    virtual void onArtistsAdded( std::vector<medialibrary::ArtistPtr> artists ) override;
    virtual void onArtistsModified( std::vector<medialibrary::ArtistPtr> artist ) override;
    virtual void onArtistsDeleted( std::vector<int64_t> ids ) override;
    virtual void onAlbumsAdded( std::vector<medialibrary::AlbumPtr> albums ) override;
    virtual void onAlbumsModified( std::vector<medialibrary::AlbumPtr> albums ) override;
    virtual void onAlbumsDeleted( std::vector<int64_t> ids ) override;
    virtual void onTracksAdded( std::vector<medialibrary::AlbumTrackPtr> tracks ) override;
    virtual void onTracksDeleted( std::vector<int64_t> trackIds ) override;
    virtual void onDiscoveryStarted( const std::string& entryPoint ) override;
    virtual void onDiscoveryCompleted( const std::string& entryPoint ) override;
    virtual void onReloadStarted( const std::string& entryPoint ) override;
    virtual void onReloadCompleted( const std::string& entryPoint ) override;
    virtual void onParsingStatsUpdated( uint32_t percent ) override;

private:
    std::unique_ptr<medialibrary::IMediaLibrary> m_ml;
};

#endif // MEDIALIBRARY_H
