/*****************************************************************************
 * Media.cpp: Represents a basic container for media informations.
 *****************************************************************************
 * Copyright (C) 2008-2010 VideoLAN
 *
 * Authors: Hugo Beauzée-Luyssen <beauze.h@gmail.com>
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
  * This file contains the Media class implementation.
  * It contains the Media meta-datas, and information to be used with a VLC Media.
  * It's used by the Library
  */

#include "Media.h"

#include "Clip.h"
#include "MetaDataManager.h"
#include "VLCMedia.h"
#include "Workspace.h"

#include <QtDebug>
#include <QUrl>

const QString   Media::VideoExtensions = "*.avi *.3gp *.amv *.asf *.divx *.dv *.flv *.gxf "
                                         "*.iso *.m1v *.m2v *.m2t *.m2ts *.m4v *.mkv *.mov "
                                         "*.mp2 *.mp4 *.mpeg *.mpeg1 *.mpeg2 *.mpeg4 *.mpg "
                                         "*.mts *.mxf *.nsv *.nuv *.ogg *.ogm *.ogv *.ogx *.ps "
                                         "*.rec *.rm *.rmvb *.tod *.ts *.vob *.vro *.webm *.wmv";

const QString   Media::ImageExtensions = "*.png *.jpg *.jpeg";

const QString   Media::AudioExtensions = "*.a52 *.aac *.ac3 *.aiff *.amr *.aob *.ape "
                                         "*.dts *.flac *.it *.m4a *.m4p *.mid *.mka *.mlp "
                                         "*.mod *.mp1 *.mp2 *.mp3 *.mpc *.oga *.ogg *.oma "
                                         "*.rmi *.s3m *.spx *.tta *.voc *.vqf *.w64 *.wav "
                                         "*.wma *.wv *.xa *.xm";
const QString   Media::streamPrefix = "stream://";

Media::Media( const QString& filePath )
    : m_vlcMedia( NULL ),
    m_fileInfo( NULL ),
    m_lengthMS( 0 ),
    m_nbFrames( 0 ),
    m_width( 0 ),
    m_height( 0 ),
    m_fps( .0f ),
    m_baseClip( NULL ),
    m_nbAudioTracks( 0 ),
    m_nbVideoTracks( 0 ),
    m_metadataComputed( false ),
    m_inWorkspace( false )
{
    if ( filePath.startsWith( Media::streamPrefix ) == false )
    {
        setFilePath( filePath );
    }
    else
    {
        m_inputType = Media::Stream;
        m_mrl = filePath.right( filePath.length() - streamPrefix.length() );
        //FIXME:
        m_fileType = Media::Video;
        m_fileName = m_mrl;
        m_vlcMedia = new LibVLCpp::Media( m_mrl );
    }
}

Media::~Media()
{
    if ( m_vlcMedia )
        delete m_vlcMedia;
    if ( m_fileInfo )
        delete m_fileInfo;
}

void
Media::computeFileType()
{
    const QString filter = "*." + m_fileInfo->suffix().toLower();
    if ( Media::VideoExtensions.contains( filter ) )
        m_fileType = Media::Video;
    else if ( Media::AudioExtensions.contains( filter ) )
        m_fileType = Media::Audio;
    else if ( Media::ImageExtensions.contains( filter ) )
        m_fileType = Media::Image;
    else
        qDebug() << "What the hell is this extension? And how did you loaded it?!";
}

const QFileInfo*
Media::fileInfo() const
{
    return m_fileInfo;
}

void
Media::setLength( qint64 length )
{
    m_lengthMS = length;
}

qint64
Media::lengthMS() const
{
    return m_lengthMS;
}

int
Media::width() const
{
    return m_width;
}

void
Media::setWidth( int width )
{
    m_width = width;
}

int
Media::height() const
{
    return m_height;
}

void
Media::setHeight( int height )
{
    m_height = height;
}

float
Media::fps() const
{
    return m_fps;
}

void
Media::setFps( float fps )
{
    m_fps = fps;
}

Media::FileType
Media::fileType() const
{
    return m_fileType;
}

void Media::setFileType(Media::FileType type)
{
    m_fileType = type;
}

void
Media::emitMetaDataComputed()
{
    m_metadataComputed = true;
    emit metaDataComputed( this );
}

Media::InputType
Media::inputType() const
{
    return m_inputType;
}

void
Media::setNbFrames( qint64 nbFrames )
{
    m_nbFrames = nbFrames;
}

qint64
Media::nbFrames() const
{
    return m_nbFrames;
}

const QString&
Media::mrl() const
{
    return m_mrl;
}

const QString&
Media::fileName() const
{
    return m_fileName;
}

bool
Media::hasAudioTrack() const
{
    return ( m_nbAudioTracks > 0 );
}

bool
Media::hasVideoTrack() const
{
    return ( m_nbVideoTracks > 0 );
}

void
Media::setNbAudioTrack( int nbTrack )
{
    m_nbAudioTracks = nbTrack;
}

void
Media::setNbVideoTrack( int nbTracks )
{
    m_nbVideoTracks = nbTracks;
}

int
Media::nbAudioTracks() const
{
    return m_nbAudioTracks;
}

int
Media::nbVideoTracks() const
{
    return m_nbVideoTracks;
}

bool
Media::isMetadataComputed() const
{
    return m_metadataComputed;
}

void
Media::save( QXmlStreamWriter& project )
{
    project.writeStartElement( "media" );
    if ( m_inWorkspace == true )
        project.writeAttribute( "mrl", Workspace::workspacePrefix + m_workspacePath );
    else
        project.writeAttribute( "mrl", m_fileInfo->absoluteFilePath() );
    project.writeEndElement();
}

void
Media::setBaseClip( Clip *clip )
{
    Q_ASSERT( m_baseClip == NULL );
    m_baseClip = clip;
}

bool
Media::isInWorkspace() const
{
    return m_inWorkspace;
}

void
Media::setFilePath( const QString &filePath )
{
    m_inputType = Media::File;
    if ( m_fileInfo )
        delete m_fileInfo;
    m_fileInfo = new QFileInfo( filePath );
    m_fileName = m_fileInfo->fileName();
    computeFileType();
    m_mrl = "file:///" + QUrl::toPercentEncoding( filePath, "/" );
    if ( m_vlcMedia )
        delete m_vlcMedia;
    m_vlcMedia = new LibVLCpp::Media( m_mrl );
    //Don't call this before setting all the internals, as it relies on Media::fileInfo.
    if ( Workspace::isInProjectDir( this ) == true )
    {
        m_inWorkspace = true;
        m_workspacePath = Workspace::pathInProjectDir( this );
    }
    else
    {
        m_inWorkspace = false;
        m_workspacePath = "";
    }
    emit workspaceStateChanged( m_inWorkspace );
}

void
Media::computeMetadata()
{
    MetaDataManager::getInstance()->computeMediaMetadata( this );
}
