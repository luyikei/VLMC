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

class Clip;
class Media;
class ProjectManager;
class Settings;

/**
 *  \class Library
 *  \brief Library Object that handles public Clips
 */
class Library : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY( Library )

public:
    Library( Settings* projectSettings );
    virtual ~Library();
    void            addMedia( QSharedPointer<Media> media );
    bool            isInCleanState() const;
    QSharedPointer<Media> media( qint64 mediaId );
    /**
     * @brief clip returns an existing clip
     * @param uuid the clip's UUID
     * @return The clip if it exists, or nullptr
     * This can be any clip, the given UUID doesn't have to refer to a root clip
     */
    Clip*           clip( const QUuid& uuid );
    void            clear();

private:
    void            setCleanState( bool newState );

private:
    bool        m_cleanState;

    Settings*   m_settings;
    QHash<qint64, QSharedPointer<Media>>  m_media;
    /**
     * @brief m_clips   contains all the clips loaded in the library, without any
     *                  subclip hierarchy
     */
    QHash<QUuid, Clip*>     m_clips;

    void        preSave();
    void        postLoad();

signals:
    /**
     *  \brief
     */
    void    cleanStateChanged( bool newState );
};

#endif // LIBRARY_H
