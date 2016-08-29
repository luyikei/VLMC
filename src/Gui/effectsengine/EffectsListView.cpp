/*****************************************************************************
 * EffectsListView.cpp: Display a list of effects
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

#include "Main/Core.h"
#include "Backend/IBackend.h"
#include "Backend/IFilter.h"
#include "EffectsListView.h"
#include "EffectWidget.h"

#include <QJsonObject>
#include <QQmlContext>
#include <QQuickView>
#include <QMimeData>
#include <QDrag>

EffectsListView::EffectsListView( QWidget* parent ) :
    QObject(parent)
{
    setObjectName( QStringLiteral( "EffectsListView" ) );
    auto view = new QQuickView;
    m_container = QWidget::createWindowContainer( view, parent );
    m_container->setMinimumSize( 100, 1 );
    m_container->setObjectName( objectName() );
    view->rootContext()->setContextProperty( QStringLiteral( "view" ), this );
    view->setSource( QUrl( QStringLiteral( "qrc:/QML/EffectsListView.qml" ) ) );
    view->setResizeMode( QQuickView::SizeRootObjectToView );
}

QWidget*
EffectsListView::container()
{
    return m_container;
}

QJsonArray
EffectsListView::effects()
{
    QJsonArray array;
    for ( auto p : Backend::instance()->availableFilters() )
    {
        auto info = p.second;
        QJsonObject jInfo;
        jInfo[QStringLiteral( "identifier" )] = QString::fromStdString( info->identifier() );
        jInfo[QStringLiteral( "name" )] = QString::fromStdString( info->name() );
        jInfo[QStringLiteral( "description" )] = QString::fromStdString( info->description() );
        jInfo[QStringLiteral( "author" )] = QString::fromStdString( info->author() );
        array.append( jInfo );
    }
    return array;
}

void
EffectsListView::startDrag( const QString& effectId )
{
    QDrag* drag = new QDrag( this );
    QMimeData* mimeData = new QMimeData;

    mimeData->setData( QStringLiteral( "vlmc/effect_name" ), effectId.toUtf8() );

    drag->setMimeData( mimeData );
    drag->exec();

}
