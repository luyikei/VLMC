/*****************************************************************************
 * MediaLibraryModel.cpp
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

#include "config.h"

#include "MediaLibraryModel.h"

#ifdef WITH_GUI
#include <QPixmap>
#endif

MediaLibraryModel::MediaLibraryModel( medialibrary::IMediaLibrary& ml, medialibrary::IMedia::Type type, QObject *parent )
    : QAbstractListModel(parent)
    , m_ml( ml )
    , m_mediaType( type )
    , m_rowCount( 0 )
{
}

void MediaLibraryModel::addMedia( medialibrary::MediaPtr media )
{
    if ( media->type() != m_mediaType )
        return;
    std::lock_guard<std::mutex> lock( m_mediaMutex );
    auto size = m_media.size();
    beginInsertRows( QModelIndex(), size, size );
    m_media.push_back( media );
    m_rowCount.fetch_add( 1, std::memory_order_relaxed );
    endInsertRows();
}

void MediaLibraryModel::updateMedia( medialibrary::MediaPtr media )
{
    auto m = createIndex( media->id(), 0 );
    emit dataChanged( m, m );
}

bool MediaLibraryModel::removeMedia( int64_t mediaId )
{
    std::lock_guard<std::mutex> lock( m_mediaMutex );
    auto it = std::find_if( begin( m_media ), end( m_media ), [mediaId](medialibrary::MediaPtr m) {
        return m->id() == mediaId;
    });
    if ( it == end( m_media ) )
        return false;
    auto idx = it - begin( m_media );
    beginRemoveRows(QModelIndex(), idx, idx );
    m_media.erase( it );
    m_rowCount.fetch_sub( 1, std::memory_order_relaxed );
    endRemoveRows();
    return true;
}

int MediaLibraryModel::rowCount( const QModelIndex& ) const
{
    return m_rowCount.load( std::memory_order_relaxed );
}

QVariant MediaLibraryModel::data( const QModelIndex &index, int role ) const
{
    medialibrary::MediaPtr m;
    {
        std::lock_guard<std::mutex> lock( m_mediaMutex );

        if ( !index.isValid() || index.row() < 0 ||
             static_cast<size_t>( index.row() ) >= m_media.size() )
            return QVariant();
        m = m_media.at( static_cast<size_t>( index.row() ) );
    }

    switch ( role )
    {
    case Qt::DisplayRole:
    case Roles::Title:
        return QVariant( QString::fromStdString( m->title() ) );
#ifdef WITH_GUI
    case Qt::DecorationRole:
        return QPixmap( QString::fromStdString( m->thumbnail() ) );
#endif
    case Roles::ThumbnailPath:
        return QVariant( QString::fromStdString( m->thumbnail() ) );
    case Roles::Duration:
        return QVariant::fromValue( m->duration() );
    case Qt::UserRole:
        return QVariant::fromValue( m );
    default:
        break;
    }

    return QVariant();
}

QHash<int, QByteArray>
MediaLibraryModel::roleNames() const
{
    return {
        { Roles::Title, "title" },
        { Roles::ThumbnailPath, "thumbnailPath" },
        { Roles::Duration, "duration" }
    };
}

void MediaLibraryModel::refresh()
{
    std::lock_guard<std::mutex> lock( m_mediaMutex );

    beginResetModel();

    switch ( m_mediaType )
    {
    case medialibrary::IMedia::Type::AudioType:
        m_media = m_ml.audioFiles();
        break;
    case medialibrary::IMedia::Type::VideoType:
        m_media = m_ml.videoFiles();
        break;
    default:
        Q_UNREACHABLE();
    }
    m_rowCount = m_media.size();
    endResetModel();
}
