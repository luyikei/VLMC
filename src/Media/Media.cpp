/*****************************************************************************
 * Media.cpp: Represents a basic container for media informations.
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
  * This file contains the Media class implementation.
  * It contains the Media meta-datas, and information to be used with a VLC Media.
  * It's used by the Library
  */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <QUrl>

#include "Media.h"

#include "Clip.h"
#include "Main/Core.h"
#include "Library/Library.h"
#include "Tools/VlmcDebug.h"
#include "Project/Workspace.h"
#include "Backend/MLT/MLTInput.h"


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

#ifdef HAVE_GUI
QPixmap*        Media::defaultSnapshot = nullptr;
#endif

Media::Media(const QString &path )
    : m_input( nullptr )
    , m_fileInfo( nullptr )
    , m_baseClip( nullptr )
{
    setFilePath( path );
}

Media::~Media()
{
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
Media::fileName() const
{
    return m_fileName;
}

void
Media::setBaseClip( Clip *clip )
{
    Q_ASSERT( m_baseClip == nullptr );
    m_baseClip = clip;
}

QVariant
Media::toVariant() const
{
    return QVariant( m_fileInfo->absoluteFilePath() );
}

Backend::IInput*
Media::input()
{
    return m_input.get();
}

const Backend::IInput*
Media::input() const
{
    return m_input.get();
}

void
Media::setFilePath( const QString &filePath )
{
    if ( m_fileInfo )
        delete m_fileInfo;
    m_fileInfo = new QFileInfo( filePath );
    m_fileName = m_fileInfo->fileName();
    m_mrl = "file:///" + QUrl::toPercentEncoding( filePath, "/" );

    m_input.reset( new Backend::MLT::MLTInput( qPrintable( filePath ) ) );
}

#ifdef HAVE_GUI
QPixmap&
Media::snapshot()
{
    if ( Media::defaultSnapshot == nullptr )
        Media::defaultSnapshot = new QPixmap( ":/images/vlmc" );

    if ( m_snapshot.isNull() == true ) {
        int height = 200;
        int width = height * m_input->aspectRatio();
        m_input->setPosition( m_input->length() / 3 );
        QImage img( m_input->image( width, height ), width, height,
                    QImage::Format_RGBA8888, []( void* buf ){ delete[] (uchar*) buf; } );
        m_input->setPosition( 0 );
        m_snapshot.convertFromImage( img );
    }

    return m_snapshot.isNull() ? *Media::defaultSnapshot : m_snapshot;
}
#endif
