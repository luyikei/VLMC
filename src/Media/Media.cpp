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

Media::Media( medialibrary::MediaPtr media, const QUuid& uuid /* = QUuid() */ )
    : m_input( nullptr )
    , m_mlMedia( media )
    , m_baseClipUuid( uuid )
    , m_baseClip( nullptr )
{
    auto files = media->files();
    Q_ASSERT( files.size() > 0 );
    for ( const auto& f : files )
    {
        if ( f->type() == medialibrary::IFile::Type::Entire ||
             f->type() == medialibrary::IFile::Type::Main )
        {
            m_mlFile = f;
            break;
        }
    }
    if ( m_mlFile == nullptr )
        vlmcFatal( "No file representing media %s", media->title().c_str(), "was found" );
    m_input.reset( new Backend::MLT::MLTInput( m_mlFile->mrl().c_str() ) );
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

qint64
Media::id() const
{
    return m_mlMedia->id();
}

Clip*
Media::baseClip()
{
    if ( m_baseClip == nullptr )
        m_baseClip = new Clip( sharedFromThis(), 0, Backend::IInput::EndOfMedia, m_baseClipUuid );
    return m_baseClip;
}

QSharedPointer<Clip>
Media::cut( qint64 begin, qint64 end )
{
    auto clip = QSharedPointer<Clip>( new Clip( sharedFromThis(), begin, end ) );
    m_clips[clip->uuid()] = clip;
    emit subclipAdded( clip );
    return clip;
}

void
Media::removeSubclip(const QUuid& uuid)
{
    if ( m_clips.remove( uuid ) == 0 )
        return;
    emit subclipRemoved( uuid );
}

QVariant
Media::toVariant() const
{
    QVariantHash h = {
        { "uuid", m_baseClip->uuid() },
        { "mlId", static_cast<qlonglong>( m_mlMedia->id() ) }
    };
    if ( m_clips.isEmpty() == false )
    {
        QVariantList l;
        for ( const auto& c : m_clips.values() )
            l << c->toVariant();
        h.insert( "clips", l );
    }
    return QVariant( h );
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

QSharedPointer<Media>
Media::fromVariant( const QVariant& v )
{
    /**
     * The media is stored as such:
     * media: {
     *  mlId: <id>   // The media library ID
     *  uuid: <uuid> // The root clip UUID
     *  clips: [
     *    <clip 1>,  // The subclips
     *    ...
     *  ]
     * }
     */
    const auto& m = v.toMap();

    if ( m.contains( "mlId" ) == false || m.contains( "uuid" ) == false )
    {
        vlmcWarning() << "Invalid clip provided:" << m << "Missing 'mlId' and/or 'uuid' field(s)";
        return {};
    }
    auto mediaId = m["mlId"].toLongLong();
    auto uuid = m["uuid"].toUuid();
    auto mlMedia = Core::instance()->mediaLibrary()->media( mediaId );
    //FIXME: Is QSharedPointer exception safe in case its constructor throws an exception?
    auto media = QSharedPointer<Media>( new Media( mlMedia, uuid ) );

    // Now load the subclips:
    if ( m.contains( "clips" ) == false )
        return media;

    auto subclips = m["clips"].toList();
    for ( const auto& c : subclips )
    {
        media->loadSubclip( c.toMap() );
    }
    return media;
}

QString
Media::snapshot()
{
    return QString::fromStdString( m_mlMedia->thumbnail() );
}

QSharedPointer<Clip>
Media::loadSubclip( const QVariantMap& m )
{
    if ( m.contains( "uuid" ) == false || m.contains( "begin" ) == false || m.contains( "end" ) == false )
    {
        vlmcWarning() << "Invalid clip provided:" << m;
        return {};
    }
    const auto& uuid = m["uuid"].toUuid();
    const auto  begin = m["begin"].toLongLong();
    const auto  end = m["end"].toLongLong();
    const auto  formats = m["formats"].toInt();
    auto clip = QSharedPointer<Clip>( new Clip( sharedFromThis(), begin, end, uuid ) );
    clip->setFormats( static_cast<Clip::Formats>( formats ) );

    m_clips[uuid] = clip;
    emit subclipAdded( clip );
    return clip;
}
