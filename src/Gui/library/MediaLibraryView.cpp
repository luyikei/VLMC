/*****************************************************************************
 * MediaLibrary.cpp: VLMC media library's ui
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "MediaLibraryView.h"

#include "Library/MediaLibrary.h"
#include "Library/MediaLibraryModel.h"
#include "Main/Core.h"

#include <QBoxLayout>
#include <QListView>
#include <QtQuick/QQuickView>
#include <QQmlContext>
#include <QUrl>

MediaLibraryView::MediaLibraryView(QWidget *parent)
    : QWidget(parent)
{
    setObjectName( QStringLiteral( "medialibrary" ) );
    auto view = new QQuickView;
    auto container = QWidget::createWindowContainer( view, this );
    auto layout = new QBoxLayout( QBoxLayout::TopToBottom, this );
    layout->addWidget( container );

    auto ctx = view->rootContext();
    ctx->setContextProperty( QStringLiteral( "mlModel" ), Core::instance()->mediaLibrary()->model( MediaLibrary::MediaType::Video ) );

    view->setSource( QUrl( QStringLiteral( "qrc:/qml/MediaLibraryView.qml" ) ) );
    view->setResizeMode(QQuickView::SizeRootObjectToView);
}

MediaLibraryView::~MediaLibraryView()
{
}
