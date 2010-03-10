/*****************************************************************************
 * MediaContainer.h: Implements the library basics
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

#ifndef MEDIACONTAINER_H
#define MEDIACONTAINER_H

#include <QHash>
#include <QObject>
#include <QUuid>

class   Media;
class   Clip;
class   QFileInfo;

class   MediaContainer : public QObject
{
    Q_OBJECT

public:
    MediaContainer( Clip* parent = NULL );
    /**
     *  \brief  returns the clip that match the unique identifier
     *  \param  uuid    the unique identifier of the media
     *  \return a pointer to the required clip, or NULL if no clips matches
     *  \sa     media( const QUuid& uuid )
     */
    Clip*   clip( const QUuid& uuid );

    /**
     *  \brief  Add an already preparsed media.
     *
     *  This will emit the newClipLoaded signal.
     *
     *  \param  media   The media to add to the library
     */
    void        addMedia( Media *media );

    /**
     *  \brief  Add a file to the media library
     *
     *  This method will also handle metadata parsing.
     *  \param  fileInfo    the file info of the media
     *  \return             true if the media was successfully loaded. false otherwise.
     *  \sa     addClip( Clip* clip )
     *  \sa     media( const QUuid& uuid)
     *  \sa     clip( const QUuid& uuid )
     */
    bool    addMedia( const QFileInfo& fileInfo, const QString& uuid = QString() );
    /**
     *  \brief  Check if a file has already been loaded into library.
     *  \param  fileInfo    The file infos
     *  \return true if the file is already loaded, false otherwhise
     */
    bool    mediaAlreadyLoaded( const QFileInfo& fileInfo );
    /**
     *  \brief  Add a clip.
     *
     *  This will emit the newClipLoaded signal.
     *
     *  \param  clip    The clip to be added.
     */
    void    addClip( Clip* clip );

    /**
     *  \return All the loaded Clip
     */
    const QHash<QUuid, Clip*>   &clips() const;

    Clip*       getParent();

    quint32     count() const;

protected:
    /**
     *  \brief The List of medias loaded into the library
     */
    QHash<QUuid, Clip*>     m_clips;

    Clip*                   m_parent;

public slots:
    /**
     *  \brief  Delete a Clip from the container
     *  \param  clip    The clip to remove.
     */
    Clip    *removeClip( const Clip* clip );
    Clip    *removeClip( const QUuid& uuid );
    /**
     *  \brief  Clear the library (remove all the loaded Clip, delete their subclips, and
     *          delete them)
     */
    void    clear();
    /**
     *  \brief      Remove all the medias, but doesn't delete them.
     */
    void    removeAll();

signals:
    /**
     *  \brief          This signal should be emitted to tell a new Clip have been added
     *  \param Clip     The newly added clip
     */
    void    newClipLoaded( Clip* );
    /**
     *  \brief This signal should be emiteted when a Clip has been removed
     *  \param uuid The removed clip uuid
     */
    void    clipRemoved( const Clip* );
};

#endif // MEDIACONTAINER_H
