/*****************************************************************************
 * Media.h : Represents a basic container for media informations.
 *****************************************************************************
 * Copyright (C) 2008-2016 VideoLAN
 *
 * Authors: Hugo Beauzée-Luyssen <hugo@beauzee.fr>
 *          Yikei Lu    <luyikei.qmltu@gmail.com>
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
  * This file contains the Media class declaration/definition.
  * It contains a ISource and some extra helpers.
  * It's used by the Library
  */

#ifndef MEDIA_H__
#define MEDIA_H__

#include "config.h"

#include <memory>

#include <QString>
#include <QObject>
#include <QXmlStreamWriter>

#include "Backend/MLT/MLTInput.h"

#include <medialibrary/IMedia.h>
#include <medialibrary/IFile.h>

#ifdef HAVE_GUI
#include <QPixmap>
#include <QImage>
#endif

namespace Backend
{
class   IInput;
namespace VLC
{
    class   VLCSource;
}
}
class Clip;

/**
  * Represents a basic container for media informations.
  */
class       Media : public QObject
{
    Q_OBJECT

public:
    /**
     *  \enum fType
     *  \brief enum to determine file type
     */
    enum    FileType
    {
        Audio,
        Video,
        Image
    };
    static const QString        VideoExtensions;
    static const QString        AudioExtensions;
    static const QString        ImageExtensions;
    static const QString        streamPrefix;

    Media( medialibrary::MediaPtr media );

    QString                     mrl() const;
    FileType                    fileType() const;
    QString                     title() const;
    qint64                      id() const;

    Clip*                       baseClip() { return m_baseClip; }
    const Clip*                 baseClip() const { return m_baseClip; }
    void                        setBaseClip( Clip* clip );

    QVariant                    toVariant() const;

    Backend::IInput*         input();
    const Backend::IInput*   input() const;

    static QSharedPointer<Media> fromVariant( const QVariant& v );

#ifdef HAVE_GUI
    // This has to be called from the GUI thread.
    QPixmap&                    snapshot();
#endif
protected:
    std::unique_ptr<Backend::IInput>         m_input;
    medialibrary::MediaPtr      m_mlMedia;
    medialibrary::FilePtr       m_mlFile;
    Clip*                       m_baseClip;

#ifdef HAVE_GUI
    static QPixmap*             defaultSnapshot;
    QPixmap                     m_snapshot;
#endif
};

#endif // MEDIA_H__
