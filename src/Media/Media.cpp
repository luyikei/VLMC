/*****************************************************************************
 * Media.cpp: Represents a basic container for media informations.
 *****************************************************************************
 * Copyright (C) 2008-2014 VideoLAN
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
  * This file contains the Media class implementation.
  * It contains the Media meta-datas, and information to be used with a VLC Media.
  * It's used by the Library
  */

#include "Media.h"

#include "Clip.h"
#include "MetaDataManager.h"
#include "VlmcDebug.h"
#include "Workspace.h"
#include "ISource.h"
#include "IBackend.h"

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

QPixmap*        Media::defaultSnapshot = NULL;

Media::Media(const QString &path )
    : m_source( NULL )
    , m_fileInfo( NULL )
    , m_baseClip( NULL )
    , m_inWorkspace( false )
    , m_snapshotImage( NULL )
{
    setFilePath( path );
}

Media::~Media()
{
    delete m_source;
    delete m_fileInfo;
}

const QFileInfo*
Media::fileInfo() const
{
    return m_fileInfo;
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

Backend::ISource*
Media::source()
{
    return m_source;
}

const Backend::ISource*
Media::source() const
{
    return m_source;
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
Media::onMetaDataComputed()
{
    emit metaDataComputed();
    if ( m_source->hasVideo() == true )
    {
        const QString filter = "*." + m_fileInfo->suffix().toLower();
        if ( Media::ImageExtensions.contains( filter ) )
            m_fileType = Image;
        else
            m_fileType = Video;
        if ( m_source->snapshot() != NULL )
        {
            Q_ASSERT( m_snapshotImage == NULL );
            m_snapshotImage = new QImage( m_source->snapshot(), 320, 180, QImage::Format_RGB32 );
            emit snapshotAvailable();
        }
    }
    else if ( m_source->hasAudio() )
        m_fileType = Audio;
    else
    {
        // We expect this case to be handled by the metadata manager. It should
        // trigger an error for this kind of file since we can't use them.
        vlmcFatal("Got metadata for a file which has no video nor audio.");
    }
}

void
Media::setFilePath( const QString &filePath )
{
    if ( m_fileInfo )
        delete m_fileInfo;
    m_fileInfo = new QFileInfo( filePath );
    m_fileName = m_fileInfo->fileName();
    m_mrl = "file:///" + QUrl::toPercentEncoding( filePath, "/" );

    Backend::IBackend* backend = Backend::getBackend();
    delete m_source;
    m_source = backend->createSource( qPrintable( filePath ) );
    MetaDataManager::getInstance()->computeMediaMetadata( this );

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

QPixmap&
Media::snapshot()
{
    if ( m_snapshot.isNull() == false )
        return m_snapshot;

    if ( m_snapshotImage != NULL )
    {
        m_snapshot = QPixmap::fromImage( *m_snapshotImage );
        delete m_snapshotImage;

        if ( m_snapshot.isNull() == false )
            return m_snapshot;
    }
    if ( Media::defaultSnapshot == NULL )
        Media::defaultSnapshot = new QPixmap( ":/images/vlmc" );
    return *Media::defaultSnapshot;
}
