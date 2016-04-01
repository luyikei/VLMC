/*****************************************************************************
 * Media.h : Represents a basic container for media informations.
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
  * This file contains the Media class declaration/definition.
  * It contains a ISource and some extra helpers.
  * It's used by the Library
  */

#ifndef MEDIA_H__
#define MEDIA_H__

#include "config.h"
#include <QString>
#include <QObject>
#include <QFileInfo>
#include <QXmlStreamWriter>

#ifdef WITH_GUI
#include <QPixmap>
#include <QImage>
#endif

namespace Backend
{
    class   ISource;
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

    Media( const QString& path );
    virtual ~Media();

    const QFileInfo             *fileInfo() const;
    const QString               &mrl() const;
    const QString               &fileName() const;
    Backend::ISource *source();
    const Backend::ISource *source() const;
    /**
     *  \brief                  Set this media's path.
     *
     *  \param      path        The media path. This should be an absolute path.
     */
    void                        setFilePath( const QString& path );

    void                        setNbFrames( qint64 nbFrames );

    qint64                      nbFrames() const;

    FileType                    fileType() const;
    void                        setFileType( FileType type );

    Clip*                       baseClip() { return m_baseClip; }
    const Clip*                 baseClip() const { return m_baseClip; }
    void                        setBaseClip( Clip* clip );

    void                        onMetaDataComputed();

    // This has to be called from the GUI thread.
    QPixmap&                    snapshot();

protected:
    Backend::ISource*           m_source;
    QString                     m_mrl;
    QFileInfo*                  m_fileInfo;
    FileType                    m_fileType;
    QString                     m_fileName;
    Clip*                       m_baseClip;

    static QPixmap*             defaultSnapshot;
    QPixmap                     m_snapshot;
    QImage*                     m_snapshotImage;

signals:
    void                        metaDataComputed();
    void                        snapshotAvailable();
};

#endif // MEDIA_H__
