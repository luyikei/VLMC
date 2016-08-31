/*****************************************************************************
 * Library.h: Multimedia library
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
 * This file the library contains class declaration/definition.
 * It's the the backend part of the Library widget of vlmc.
 * It can load and unload Medias (Medias.h/Media.cpp)
 * It can load and unload Clips (Clip.h/Clip.cpp)
 */

#ifndef LIBRARY_H
#define LIBRARY_H

#include <QObject>
#include <QHash>
#include <QSharedPointer>

#include <medialibrary/IMediaLibrary.h>

#include <memory>

class Clip;
class Media;
class MediaLibraryModel;
class ProjectManager;
class Settings;

/**
 *  \class Library
 *  \brief Library Object that handles public Clips
 */
class Library : public QObject, private medialibrary::IMediaLibraryCb
{
    Q_OBJECT
    Q_DISABLE_COPY( Library )

public:
    enum class MediaType
    {
        Video,
        Audio
    };

    Library( Settings* vlmcSettings, Settings* projectSettings );
    virtual ~Library();
    void            addMedia( QSharedPointer<Media> media );
    bool            isInCleanState() const;
    QSharedPointer<Media> media( qint64 mediaId );

    //FIXME: This feels rather ugly
    medialibrary::MediaPtr mlMedia( qint64 mediaId);

    MediaLibraryModel* model( MediaType type ) const;

    /**
     * @brief clip returns an existing clip
     * @param uuid the clip's UUID
     * @return The clip if it exists, or nullptr
     * This can be any clip, the given UUID doesn't have to refer to a root clip
     */
    QSharedPointer<Clip>        clip( const QUuid& uuid );
    void            clear();

private:
    void            setCleanState( bool newState );
    void            mlDirsChanged( const QVariant& value );
    void            workspaceChanged(const QVariant& workspace );

    void            preSave();
    void            postLoad();

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
    virtual void onDiscoveryProgress( const std::string& entryPoint ) override;
    virtual void onDiscoveryCompleted( const std::string& entryPoint ) override;
    virtual void onParsingStatsUpdated( uint32_t percent ) override;

private:
    std::unique_ptr<medialibrary::IMediaLibrary>    m_ml;
    MediaLibraryModel*                              m_videoModel;
    MediaLibraryModel*                              m_audioModel;
    Settings*                                       m_settings;
    bool                                            m_initialized;
    bool                                            m_cleanState;

    QHash<qint64, QSharedPointer<Media>>            m_media;
    /**
     * @brief m_clips   contains all the clips loaded in the library, without any
     *                  subclip hierarchy
     */
    QHash<QUuid, QSharedPointer<Clip>>              m_clips;

signals:
    /**
     *  \brief
     */
    void    cleanStateChanged( bool newState );

    void    progressUpdated( int percent );
    void    discoveryProgress( QString );
    void    discoveryCompleted( QString );

};

#endif // LIBRARY_H
