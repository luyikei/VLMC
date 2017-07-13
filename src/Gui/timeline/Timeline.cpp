/*****************************************************************************
 * Timeline.cpp: Widget that handle the tracks
 *****************************************************************************
 * Copyright (C) 2008-2016 VideoLAN
 *
 * Authors: Ludovic Fauvet <etix@l0cal.com>
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "Timeline.h"

#include "Commands/Commands.h"
#include "Main/Core.h"
#include "Workflow/MainWorkflow.h"
#include "Gui/MainWindow.h"
#include "Gui/effectsengine/EffectStack.h"
#include "ThumbnailImageProvider.h"
#include "Settings/Settings.h"
#include "Tools/VlmcDebug.h"
#include "MarkerManager.h"

#include <QtQuick/QQuickView>
#include <QtQml/QQmlContext>
#include <QUrl>

Timeline::Timeline( Settings* projectSettings, MainWindow* parent )
    : QObject( parent )
    , m_view( new QQuickView )
    , m_container( QWidget::createWindowContainer( m_view, parent ) )
    , m_markerManager( new MarkerManager )
    , m_settings( new Settings )
{
    m_container->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    m_container->setFocusPolicy( Qt::TabFocus );
    auto p = new ThumbnailImageProvider;
    m_view->engine()->addImageProvider( QStringLiteral( "thumbnail" ), p );
    m_view->rootContext()->setContextProperty( QStringLiteral( "thumbnailProvider" ), p );
    m_view->rootContext()->setContextProperty( QStringLiteral( "timeline" ), this );
    m_view->rootContext()->setContextProperty( QStringLiteral( "mainwindow" ), parent );
    m_view->rootContext()->setContextProperty( QStringLiteral( "workflow" ), Core::instance()->workflow() );
    m_view->setSource( QUrl( QStringLiteral( "qrc:/QML/main.qml" ) ) );

    connect( Core::instance()->workflow(), &MainWorkflow::cleared, this, [this]()
    {
        m_view->setSource( QUrl() );
        m_view->engine()->clearComponentCache();
        m_view->setSource( QUrl( QStringLiteral( "qrc:/QML/main.qml" ) ) );
    } );
    connect( m_markerManager.data(), &MarkerManager::markerAdded, this, &Timeline::markerAdded );
    connect( m_markerManager.data(), &MarkerManager::markerMoved, this, &Timeline::markerMoved );
    connect( m_markerManager.data(), &MarkerManager::markerRemoved, this, &Timeline::markerRemoved );

    m_settings->createVar( SettingValue::List, QStringLiteral( "markers" ), QVariantList(),
                           "Markers", "List of markers that the timeline uses",
                           SettingValue::Folders );
    connect( m_settings.get(), &Settings::postLoad, this, &Timeline::postLoad, Qt::DirectConnection );
    connect( m_settings.get(), &Settings::preSave, this, &Timeline::preSave, Qt::DirectConnection );
    projectSettings->addSettings( QStringLiteral( "Timeline" ), *m_settings );
}

Timeline::~Timeline()
{
    delete m_view;
}

QWidget*
Timeline::container()
{
    return m_container;
}

void
Timeline::addMarker( quint64 pos )
{
    Core::instance()->workflow()->trigger( new Commands::Marker::Add( m_markerManager, pos ) );
}

void
Timeline::moveMarker( quint64 from, quint64 to )
{
    Core::instance()->workflow()->trigger( new Commands::Marker::Move( m_markerManager, from, to ) );
}

void
Timeline::removeMarker( quint64 pos )
{
    Core::instance()->workflow()->trigger( new Commands::Marker::Remove( m_markerManager, pos ) );
}

void
Timeline::preSave()
{
    m_settings->value( QStringLiteral( "markers" ) )->set( m_markerManager->toVariant() );
}

void
Timeline::postLoad()
{
    m_markerManager->fromVariant( m_settings->value( QStringLiteral( "markers" ) )->get() );
}
