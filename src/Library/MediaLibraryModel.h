/*****************************************************************************
 * MediaLibraryModel.h
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

#ifndef MEDIALIBRARYMODEL_H
#define MEDIALIBRARYMODEL_H

#include <QAbstractListModel>

#include <atomic>
#include <mutex>

#include <medialibrary/IMediaLibrary.h>
#include <medialibrary/IMedia.h>

class MediaLibraryModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles
    {
        Title = Qt::UserRole + 1,
        ThumbnailPath,
        Duration,
        Id,
    };

    explicit MediaLibraryModel( medialibrary::IMediaLibrary& ml, QObject *parent = 0 );

    medialibrary::MediaPtr findMedia( qint64 mediaId );

    virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    virtual QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    virtual QHash<int, QByteArray> roleNames() const override;

public slots:
    void addMedia( medialibrary::MediaPtr media );
    void updateMedia( medialibrary::MediaPtr media );
    bool removeMedia( int64_t media );
    void refresh();

private:
    medialibrary::IMediaLibrary& m_ml;
    std::vector<medialibrary::MediaPtr> m_media;
};

Q_DECLARE_METATYPE( medialibrary::MediaPtr );

#endif // MEDIALIBRARYMODEL_H
