/*****************************************************************************
 * main.cpp: VLMC main
 *****************************************************************************
 * Copyright (C) 2008-2009 the VLMC team
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

/** \file
 *  This file contain the main function.
 *  It will initialize the QT application,
 *  and start it.
 */

#include "config.h"
#include "MainWindow.h"
#include "project/GuiProjectManager.h"
#include "IntroDialog.h"
#include "LanguageHelper.h"

#include <QApplication>
#include <QSettings>
#include <QFile>
#include <QColor>
#include <QPalette>

/**
 *  VLMC Entry point
 *  \brief this is the VLMC entry point
 *  \param argc
 *  \param argv
 *  \return Return value of vlmc
 */
int
VLMCmain( int argc, char **argv )
{
    QApplication app( argc, argv );
    app.setApplicationName( "vlmc" );
    app.setOrganizationName( "vlmc" );
    app.setOrganizationDomain( "vlmc.org" );
    app.setApplicationVersion( PROJECT_VERSION );

    QSettings s;
    LanguageHelper::getInstance()->languageChanged(
            s.value( "general/VLMCLang", "default" ) );

#ifdef Q_OS_WIN

    QFile  css(":/styles/windows");
    if ( css.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
        QString styleSheet = css.readAll();
        if ( styleSheet != "" )
            app.setStyleSheet( styleSheet );
    }
#endif

    // Creating the color palette
    QPalette p;
    p.setColor( QPalette::WindowText,       QColor( 255, 255, 255, 255 ) );
    p.setColor( QPalette::Button,           QColor( 104, 103, 103, 255 ) );
    p.setColor( QPalette::Light,            QColor( 156, 155, 155, 255 ) );
    p.setColor( QPalette::Midlight,         QColor( 130, 129, 129, 255 ) );
    p.setColor( QPalette::Dark,             QColor( 52,  51,  51,  255 ) );
    p.setColor( QPalette::Mid,              QColor( 69,  68,  68,  255 ) );
    p.setColor( QPalette::Text,             QColor( 255, 255, 255, 255 ) );
    p.setColor( QPalette::BrightText,       QColor( 255, 255, 255, 255 ) );
    p.setColor( QPalette::ButtonText,       QColor( 255, 255, 255, 255 ) );
    p.setColor( QPalette::Base,             QColor( 104, 103, 103, 255 ) );
    p.setColor( QPalette::Window,           QColor( 73,  72,  72,  255 ) );
    p.setColor( QPalette::Shadow,           QColor( 0,   0,   0,   255 ) );
    p.setColor( QPalette::AlternateBase,    QColor( 52,  51,  51,  255 ) );
    p.setColor( QPalette::ToolTipBase,      QColor( 255, 255, 220, 255 ) );
    p.setColor( QPalette::ToolTipText,      QColor( 0,   0,   0,   255 ) );
    p.setColor( QPalette::WindowText,       QColor( 255, 255, 255, 255 ) );
    p.setColor( QPalette::WindowText,       QColor( 255, 255, 255, 255 ) );
    p.setColor( QPalette::WindowText,       QColor( 255, 255, 255, 255 ) );
    p.setColor( QPalette::WindowText,       QColor( 255, 255, 255, 255 ) );
    p.setColor( QPalette::Link,             QColor( 177, 202, 0,   255 ) );
    p.setColor( QPalette::LinkVisited,      QColor( 177, 202, 0,   255 ) );

#ifndef QT_DEBUG
    IntroDialog d;
    d.exec();
#endif

    app.setPalette( p );

    MainWindow w;
    if ( argc > 1 )
        GUIProjectManager::getInstance()->ProjectManager::loadProject( argv[argc - 1] );
    w.show();
    return app.exec();
}
