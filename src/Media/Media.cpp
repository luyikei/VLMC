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
#include <QVariant>
#include <QFileInfo>

#include "Media.h"

#include "Clip.h"
#include "Main/Core.h"
#include "Library/Library.h"
#include "Library/MediaLibrary.h"
#include "Tools/VlmcDebug.h"
#include "Project/Workspace.h"

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

Media::Media( medialibrary::MediaPtr media )
    : m_input( nullptr )
    , m_mlMedia( media )
    , m_baseClip( nullptr )
{
    auto files = media->files();
    Q_ASSERT( files.size() > 0 );
    for ( const auto& f : files )
    {
        if ( f->type() == medialibrary::IFile::Type::Main )
        {
            m_mlFile = f;
            break;
        }
    }
    if ( m_mlFile == nullptr )
        vlmcCritical() << "No file representing media", media->title(), "was found";
}

QString
Media::mrl() const
{
    return QString::fromStdString( m_mlFile->mrl() );
}

Media::FileType
Media::fileType() const
{
    switch ( m_mlMedia->type() )
    {
    case medialibrary::IMedia::Type::VideoType:
        return Video;
    case medialibrary::IMedia::Type::AudioType:
        return Audio;
    //FIXME: Unhandled Image type
    default:
        vlmcCritical() << "Unknown file type";
    }
}

QString
Media::title() const
{
    return QString::fromStdString( m_mlMedia->title() );
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
    return QVariant( static_cast<qlonglong>( m_mlMedia->id() ) );
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

Media* Media::fromVariant( const QVariant& v )
{
    bool ok = false;
    auto mediaId = v.toLongLong( &ok );
    if ( ok == false )
        return nullptr;
    auto mlMedia = Core::instance()->mediaLibrary()->media( mediaId );
    return new Media( mlMedia );
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
