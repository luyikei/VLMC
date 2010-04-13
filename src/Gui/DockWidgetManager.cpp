/*****************************************************************************
 * DockWidgetManager.cpp: Object managing the application docked widget
 *****************************************************************************
 * Copyright (C) 2008-2010 VideoLAN
 *
 * Authors: Clement CHAVANCE <chavance.c@gmail.com>
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
#include <QtDebug>
#include <QApplication>
#include <QMap>
#include <QMapIterator>
#include <QDockWidget>

#include "DockWidgetManager.h"
#include "MainWindow.h"

DockWidgetManager::DockWidgetManager( QObject *parent )
    : QObject( parent )
{
    QObject::connect( qApp,
                      SIGNAL( aboutToQuit() ),
                      this,
                      SLOT( deleteLater() ) );
}

DockWidgetManager::~DockWidgetManager()
{
    QList<QDockWidget*> widgets = m_dockWidgets.values();
    QDockWidget* dockWidget;

    foreach( dockWidget, widgets )
        delete dockWidget;
}

void DockWidgetManager::setMainWindow( MainWindow *mainWin )
{
    m_mainWin = mainWin;
}

QDockWidget* DockWidgetManager::addDockedWidget( QWidget *widget,
                                       const char *qs_name,
                                       Qt::DockWidgetAreas areas,
                                       QDockWidget::DockWidgetFeature features,
                                       Qt::DockWidgetArea startArea)
{
    if ( m_dockWidgets.contains( qs_name ) )
        return 0;

    QDockWidget* dock = new QDockWidget( tr( qs_name ), m_mainWin );

    m_dockWidgets.insert( qs_name, dock );
    dock->setWidget( widget );
    dock->setAllowedAreas( areas );
    dock->setFeatures( features );
    m_mainWin->addDockWidget( startArea, dock );
    m_mainWin->registerWidgetInWindowMenu( dock );
    widget->show();
    return dock;
}

void DockWidgetManager::retranslateUi()
{
    QMapIterator<const char*, QDockWidget*> i( m_dockWidgets );

    while ( i.hasNext() )
    {
        i.next();
        i.value()->setWindowTitle( tr( i.key() ) );
    }
}
