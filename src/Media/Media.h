/*****************************************************************************
 * Media.h : Represents a basic container for media informations.
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
  * This file contains the Media class declaration/definition.
  * It contains a VLCMedia and the meta-datas.
  * It's used by the Library
  */

#ifndef MEDIA_H__
#define MEDIA_H__

#include "config.h"
#include <QList>
#include <QString>
#include <QUuid>
#include <QObject>
#include <QFileInfo>
#include <QHash>
#include <QXmlStreamWriter>

#ifdef WITH_GUI
#include "media/GuiMedia.h"
#endif

namespace LibVLCpp
{
    class   Media;
}
class Clip;

/**
  * Represents a basic container for media informations.
  */
#ifdef WITH_GUI
class       Media : public GUIMedia
#else
class       Media : public QObject
#endif
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
    enum    InputType
    {
        File,
        Stream
    };
    Media( const QString& filePath );
    virtual ~Media();

    /**
     *  \brief  This method adds a parameter that will stay constant though the whole life
     *          of this media (unless it is explicitely overided), even if it is cloned.
     */
    void                        addConstantParam( const QString& param );
    /**
     *  \brief  This method will add a parameter that will be restored to defaultValue when the flushVolatileParameter is called
     */
    void                        addVolatileParam( const QString& param, const QString& defaultValue );
    void                        flushVolatileParameters();
    LibVLCpp::Media             *vlcMedia() { return m_vlcMedia; }

    const QFileInfo             *fileInfo() const;
    const QString               &mrl() const;
    const QString               &fileName() const;
    void                        setFilePath( const QString& path );

    /**
        \return                 Returns the length of this media (ie the
                                video duration) in milliseconds.
    */
    qint64                      lengthMS() const;
    /**
        \brief                  This methods is most of an entry point for the
                                MetadataManager than enything else.
                                If you use it to set a inconsistant media length
                                you'll just have to blame yourself !
    */
    void                        setLength( qint64 length );
    void                        setNbFrames( qint64 nbFrames );

    int                         width() const;
    void                        setWidth( int width );

    int                         height() const;
    void                        setHeight( int height );

    float                       fps() const;
    void                        setFps( float fps );

    qint64                      nbFrames() const;

    bool                        hasAudioTrack() const;
    bool                        hasVideoTrack() const;
    void                        setNbAudioTrack( int nbTrack );
    void                        setNbVideoTrack( int nbTrack );
    int                         nbAudioTracks() const;
    int                         nbVideoTracks() const;

    FileType                    fileType() const;
    static const QString        VideoExtensions;
    static const QString        AudioExtensions;
    static const QString        ImageExtensions;
    InputType                   inputType() const;
    static const QString        streamPrefix;

    void                        emitMetaDataComputed();
    void                        emitAudioSpectrumComuted();

    QList<int>*                 audioValues() { return m_audioValueList; }

    Clip*                       baseClip() { return m_baseClip; }
    const Clip*                 baseClip() const { return m_baseClip; }
    void                        setBaseClip( Clip* clip );

    bool                        isMetadataComputed() const;

    void                        save( QXmlStreamWriter& project );

    bool                        isInWorkspace() const;
    void                        setInWorkspace( bool inWorkspace );

private:
    void                        setFileType();

protected:
    LibVLCpp::Media*            m_vlcMedia;
    QString                     m_mrl;
    QList<QString>              m_volatileParameters;
    QFileInfo*                  m_fileInfo;
    qint64                      m_lengthMS;
    qint64                      m_nbFrames;
    unsigned int                m_width;
    unsigned int                m_height;
    float                       m_fps;
    FileType                    m_fileType;
    InputType                   m_inputType;
    QString                     m_fileName;
    Clip*                       m_baseClip;
    QList<int>*                 m_audioValueList;
    int                         m_nbAudioTracks;
    int                         m_nbVideoTracks;
    bool                        m_metadataComputed;
    bool                        m_inWorkspace;

signals:
    void                        metaDataComputed( const Media* );
    void                        audioSpectrumComputed( const QUuid& );
};

#endif // MEDIA_H__
