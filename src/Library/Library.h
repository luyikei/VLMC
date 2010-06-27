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
#include "MediaContainer.h"

#include <QObject>
#include <QAtomicInt>
#include <QXmlStreamWriter>

class   QDomElement;

class Clip;
class Media;

/**
 *  \class Library
 *  \brief Library Object that handles public Clips
 */
class Library : public MediaContainer, public Singleton<Library>
{
    Q_OBJECT
    Q_DISABLE_COPY( Library );

public:
    virtual void    addMedia( Media* media );
    virtual Media   *addMedia( const QFileInfo &fileInfo );
    virtual bool    addClip( Clip *clip );
    bool            isInCleanState() const;

private:
    Library();
    virtual ~Library(){}

    void        setCleanState( bool newState );

private:
    QAtomicInt  m_nbMediaToLoad;
    bool        m_cleanState;

public slots:
    /**
     *  \brief
     */
    void    loadProject( const QDomElement& project );
    /**
     *  \brief
     */
    void    saveProject( QXmlStreamWriter& project );

private slots:
    void    mediaLoaded( const Media* m );

signals:
    /**
     *  \brief
     */
    void    projectLoaded();
    void    cleanStateChanged( bool newState );

    friend class    Singleton<Library>;
    friend class    Workspace;
};

#endif // LIBRARY_H
