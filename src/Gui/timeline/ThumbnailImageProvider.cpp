/*****************************************************************************
 * ThumbnailImageProvider.cpp
 *****************************************************************************
 * Copyright (C) 2008-2016 VideoLAN
 *
 * Authors: Yikei Lu    <luyikei.qmltu@gmail.com>
 *          Hugo Beauzée-Luyssen <hugo@beauzee.fr>
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "ThumbnailImageProvider.h"

#include "Library/Library.h"
#include "Media/Clip.h"
#include "Media/Media.h"
#include "Main/Core.h"
#include "Tools/VlmcDebug.h"
#include "Workflow/MainWorkflow.h"

ThumbnailImageProvider::ThumbnailImageProvider()
    : QQuickImageProvider( QQuickImageProvider::Image )
{
}

QImage
ThumbnailImageProvider::requestImage( const QString& id, QSize* size, const QSize& requestedSize )
{
    QString tmp = id;
    tmp.replace( "%7B", "{" );
    tmp.replace( "%7D", "}" );

    auto infos = tmp.split( '/' );
    auto libraryUuid = infos[0];
    auto clip = Core::instance()->library()->clip( libraryUuid );
    QImage snapshot( clip->media()->snapshot() );
    *size = snapshot.size();
    auto width = requestedSize.width() > 0 ? requestedSize.width() : size->width();
    auto height = requestedSize.height() > 0 ? requestedSize.height() : size->height();
    return snapshot.scaled( width, height );
}
