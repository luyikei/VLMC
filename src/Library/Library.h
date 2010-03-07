/*****************************************************************************
 * Library.h: Multimedia library
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
 * This file the library contains class declaration/definition.
 * It's the the backend part of the Library widget of vlmc.
 * It can load and unload Medias (Medias.h/Media.cpp)
 * It can load and unload Clips (Clip.h/Clip.cpp)
 */

#ifndef LIBRARY_H
#define LIBRARY_H

#include "Singleton.hpp"

#include <QHash>
#include <QObject>
#include <QUuid>
#include <QFileInfo>
#include <QMutex>
#include <QMutexLocker>
#include <QDomElement>
#include <QProgressDialog>

class QDomDocument;
class QDomElement;

class Clip;
class Media;

/**
 *  \class Library
 *  \brief Library Object that handles medias
 */
class Library : public QObject, public Singleton<Library>
{
    Q_OBJECT
    Q_DISABLE_COPY( Library );
public:
    /**
     *  \brief  returns the media that match the unique identifier
     *  \param  uuid    the unique identifier of the media
     *  \return a pointer to the required media, or NULL if no medias matches
     *  \sa     clip( const QUuid& uuid )
     */
    Media*  media( const QUuid& uuid );
    /**
     *  \brief  returns the clip that match the unique identifier
     *  \param  uuid    the unique identifier of the media
     *  \return a pointer to the required clip, or NULL if no clips matches
     *  \sa     media( const QUuid& uuid )
     */
    Clip*   clip( const QUuid& uuid );
    /**
     *  \brief  returns the clip that match the unique identifier
     *  \param  mediaUuid the unique identifier of the media
     *  \param  clipUuid the unique identifier of the clip
     *  \return a pointer to the required clip, or NULL if no clips matches
     *  \sa     media( const QUuid& uuid )
     */
    Clip*   clip( const QUuid& mediaUuid, const QUuid& clipUuid );

    /**
     *  \brief  Add a file to the media library
     *
     *  This method will also handle metadata parsing. Right now, this is only meant to be
     *  used when loading a project.
     *  \param  fileInfo    the file info of the media
     *  \return             true if the media was successfully loaded. false otherwise.
     *  \sa     addClip( Clip* clip )
     *  \sa     media( const QUuid& uuid)
     *  \sa     clip( const QUuid& uuid )
     */
    bool    addMedia( const QFileInfo& fileInfo, const QString& uuid = QString() );

    /**
     *  \brief  Add an already preparsed media.
     *
     *  \param  media   The media to add to the library
     */
    void    addMedia( Media *media );

    /**
     *  \brief  Check if a file has already been loaded into library.
     *  \param  fileInfo    The file infos
     *  \return true if the file is already loaded, false otherwhise
     */
    bool    mediaAlreadyLoaded( const QFileInfo& fileInfo );

private:
    /**
     *  \brief Library Object Constructor
     */
    Library();
    /**
     *  \brief  This method is used to load a media directly from it's
     *          path, with a specified UUID.
     *          It shouldn't used for something else that loading a project file
     *  \param  path The path of the media file
     *  \param  uuid The uuid you want for the new media
     */

    /**
     *  \brief The List of medias loaded into the library
     */
    QHash<QUuid, Media*>    m_medias;

public slots:
    /**
     *  \brief This slot must be called when you want a media to be removed from library
     *  \param uuid The uuid of the media to be removed
     */
    void    removingMediaAsked( const QUuid& uuid );
    /**
     *  \brief  Delete the media when it's ready
     *  \param  uuid the unique identifier of the media to delete
     */
    void    deleteMedia( const QUuid& uuid );
    /**
     *  \brief
     */
    void    loadProject( const QDomElement& project );
    /**
     *  \brief
     */
    void    saveProject( QDomDocument& doc, QDomElement& rootNode );
    /**
     *  \brief  Clear the library (remove all the loaded media and delete them)
     */
    void    clear();
    /**
     *  \brief Remove a clip from a media
     */
    void    removeClip( const QUuid& mediaId, const QUuid& clipId );

signals:
    /**
     *  \brief          This signal should be emitted to tell a new media have been added
     *  \param media    The newly added media
     */
    void    newMediaLoaded( Media* );
    /**
     *  \brief This signal should be emiteted when a media has been removed
     *  \param uuid The removed media uuid
     */
    void    mediaRemoved( const QUuid& );
    /**
     *  \brief
     */
    void    projectLoaded();
    /**
     *  \brief
     */
    void    updateMediaRequested( const QUuid& uuid );
    friend class    Singleton<Library>;
};

#endif // LIBRARY_H
