/*****************************************************************************
 * MediaContainer.h: Implements the library basics
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
    MediaContainer( Clip* parent = nullptr );
    ~MediaContainer();
    /**
     *  \brief  returns the clip that match the unique identifier
     *  \param  uuid    the unique identifier of the media
     *  \return a pointer to the required clip, or nullptr if no clips matches
     */
    Clip*   clip( const QUuid& uuid );

    /**
     *  \brief  returns the clip that match the unique identifier
     *
     *  The identifier may be a full id (ie the full path, starting at the root clip)
     *  \param  uuid    the unique identifier of the media
     *  \return a pointer to the required clip, or nullptr if no clips matches
     */
    Clip*   clip( const QString& uuid );

    /**
     *  \brief  Add an already preparsed media.
     *
     *  This will emit the newClipLoaded signal.
     *
     *  \param  media   The media to add to the library
     */
    virtual void        addMedia( Media *media );

    /**
     *  \brief  Add a file to the media library
     *
     *  This method will also handle metadata parsing.
     *  \param  fileInfo    the file info of the media
     *  \return             The newly create media if the media was successfully loaded.
     *                      nullptr otherwise.
     *  \sa     addClip( Clip* clip )
     *  \sa     media( const QUuid& uuid)
     *  \sa     clip( const QUuid& uuid )
     */
    virtual Media       *addMedia( const QFileInfo& fileInfo );
    /**
     *  \brief  Check if a file has already been loaded into library.
     *  \param  fileInfo    The file infos
     *  \return true if the file is already loaded, false otherwhise
     */
    bool        mediaAlreadyLoaded( const QFileInfo& fileInfo );
    /**
     *  \brief  Add a clip.
     *
     *  The method will first check for an identic clip existence.
     *  This will emit the newClipLoaded signal if the Clip is added.
     *
     *  \param  clip    The clip to be added.
     *  \return true if the Clip has been added.
     */
    virtual bool    addClip( Clip* clip );

    /**
     *  \return All the loaded Clip
     */
    const QHash<QUuid, Clip*>   &clips() const;

    /**
     *  \breif  Emit newClipLoaded from all clips
     */
    void                        reloadAllClips();

    Clip*       getParent();

    quint32     count() const;

protected:
    /**
     *  \brief The List of medias loaded into the library
     */
    QHash<QUuid, Clip*>     m_clips;

    /**
     *  \brief  Used when loading a project.
     *
     *  This should not have an associated getter.
     */
    QHash<QString, Media*>  m_medias;

    Clip*                   m_parent;

public slots:
    /**
     *  \brief  Removes a Clip from the container and delete it
     *
     *  \param  uuid    The clip to remove's uuid.
     */
    void    deleteClip( const QUuid& uuid );
    /**
     *  \brief  Clear the library (remove all the loaded Clip, delete their subclips, and
     *          delete them)
     */
    void    clear();
    /**
     *  \brief      Remove all the medias from the container, but doesn't clean nor
     *              delete them.
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
     *  This signal pass a QUuid as the clip may be deleted when the signal reaches its
     *  slot.
     *  \param uuid The removed clip uuid
     */
    void    clipRemoved( const QUuid& );
};

#endif // MEDIACONTAINER_H
