/*****************************************************************************
 * Timeline.cpp: Widget that handle the tracks
 *****************************************************************************
 * Copyright (C) 2008-2016 VideoLAN
 *
 * Authors: Ludovic Fauvet <etix@l0cal.com>
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

#include "Timeline.h"

#include "Main/Core.h"
#include "Workflow/MainWorkflow.h"
#include "Gui/MainWindow.h"
#include <QtQuick/QQuickView>
#include <QtQml/QQmlContext>
#include <QUrl>

Timeline*   Timeline::m_instance = nullptr;

Timeline::Timeline( MainWindow* parent )
    : QObject( parent )
    , m_view( new QQuickView )
    , m_container( QWidget::createWindowContainer( m_view, parent ) )
{
    m_container->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    m_container->setFocusPolicy( Qt::TabFocus );
    m_view->rootContext()->setContextProperty( "mainwindow", parent );
    m_view->rootContext()->setContextProperty( "workflow", Core::instance()->workflow() );
    m_view->setSource( QUrl( QStringLiteral( "qrc:/QML/main.qml" ) ) );
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
