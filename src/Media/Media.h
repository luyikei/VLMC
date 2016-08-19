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

#include <QEnableSharedFromThis>
#include <QHash>
#include <QString>
#include <QObject>
#include <QUuid>
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
class       Media : public QObject, public QEnableSharedFromThis<Media>
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

    Media( medialibrary::MediaPtr media, const QUuid& uuid = QUuid() );

    QString                     mrl() const;
    FileType                    fileType() const;
    QString                     title() const;
    qint64                      id() const;

    Clip*                       baseClip() { return m_baseClip; }
    const Clip*                 baseClip() const { return m_baseClip; }
    /**
     * @brief cut   Creates a clip to represent a cut of a media
     * @param begin The first frame of the cut
     * @param end   The last frame of the cut
     * @return      A new Clip, representing the media from [begin] to [end]
     */
    QSharedPointer<Clip>        cut( qint64 begin, qint64 end );
    void                        removeSubclip( const QUuid& uuid );

    QVariant                    toVariant() const;

    Backend::IInput*         input();
    const Backend::IInput*   input() const;

    static QSharedPointer<Media> fromVariant( const QVariant& v );

#ifdef HAVE_GUI
    // This has to be called from the GUI thread.
    QPixmap&                    snapshot();
#endif

private:
    QSharedPointer<Clip>        loadSubclip( const QVariantMap& m );

protected:
    std::unique_ptr<Backend::IInput>         m_input;
    medialibrary::MediaPtr      m_mlMedia;
    medialibrary::FilePtr       m_mlFile;
    Clip*                       m_baseClip;
    QHash<QUuid, QSharedPointer<Clip>>      m_clips;

#ifdef HAVE_GUI
    static QPixmap*             defaultSnapshot;
    QPixmap                     m_snapshot;
#endif

signals:
    /**
     *  \brief          This signal should be emitted to tell a new sublip have been added
     *  \param Clip     The newly added subclip
     */
    void    subclipAdded( QSharedPointer<Clip> );
    /**
     *  \brief This signal should be emiteted when a subclip has been removed
     *  This signal pass a QUuid as the clip may be deleted when the signal reaches its
     *  slot.
     *  \param uuid The removed clip uuid
     */
    void    subclipRemoved( const QUuid& );
};

#endif // MEDIA_H__
