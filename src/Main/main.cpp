/*****************************************************************************
 * main.cpp: VLMC main
 *****************************************************************************
 * Copyright (C) 2008-2016 the VLMC team
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

#include "Tools/VlmcDebug.h"
#include "Workflow/Types.h"
#include "Renderer/ConsoleRenderer.h"
#include "Project/Project.h"
#include "Backend/IBackend.h"
#include "Main/Core.h"
#include "Settings/Settings.h"
#ifdef HAVE_GUI
#include "Gui/MainWindow.h"
#include "Gui/IntroDialog.h"
#include "Gui/LanguageHelper.h"
#include "Gui/wizard/firstlaunch/FirstLaunchWizard.h"
#endif

#ifdef HAVE_GUI
#include <QApplication>
#include <QColor>
#include <QPalette>
#else
#include <QCoreApplication>
#endif
#include <QFile>
#include <QSettings>
#include <QUuid>
#include <QTextCodec>

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#endif

static void
VLMCmainCommon( const QCoreApplication &app, Backend::IBackend** backend )
{
    app.setApplicationName( "vlmc" );
    app.setOrganizationName( "VideoLAN" );
    app.setOrganizationDomain( "videolan.org" );
    app.setApplicationVersion( PACKAGE_VERSION );

    QSettings s;

    qRegisterMetaType<Workflow::TrackType>( "Workflow::TrackType" );
    qRegisterMetaType<Vlmc::FrameChangedReason>( "Vlmc::FrameChangedReason" );
    qRegisterMetaType<QVariant>( "QVariant" );
    qRegisterMetaType<QUuid>( "QUuid" );

    *backend = Backend::instance();
}

/**
 *  VLMC Entry point
 *  \brief this is the VLMC entry point
 *  \param argc
 *  \param argv
 *  \return Return value of vlmc
 */
#ifdef HAVE_GUI
int
VLMCGuimain( int argc, char **argv )
{
#ifdef Q_WS_X11
    XInitThreads();
#endif

    QApplication app( argc, argv );

    Backend::IBackend* backend;
    VLMCmainCommon( app, &backend );
    auto coreLock = Core::Policy_t::lock();

    /* Load a project file */
    bool        project = false;
    for ( int i = 1; i < argc; i++ )
    {
        QString arg = argv[i];

        if ( argc > ( i + 1 ) && ( arg == "--project" || arg == "-p" ) )
        {
            Core::instance()->loadProject( argv[i+1] );
            project = true;
            break;
        }
    }

    /* Translations */
    QSettings s;
    LanguageHelper::instance()->languageChanged(
            s.value( "vlmc/VLMCLang", "default" ) );

#if defined( Q_WS_WIN )
    QFile  css(":/styles/windows");
    if ( css.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
        QString styleSheet = css.readAll();
        if ( styleSheet != "" )
            app.setStyleSheet( styleSheet );
    }

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
    app.setPalette( p );
#endif

#ifndef QT_DEBUG
    IntroDialog d;
    d.exec();
#endif

    MainWindow w( backend );

    if ( FirstLaunchWizard::shouldRun() == true )
    {
        FirstLaunchWizard wiz;
        if ( wiz.exec() == QDialog::Rejected )
            return 1;
    }

    //Don't show the wizard if a project has been passed through command line.
    if ( project == false )
        w.showWizard();

    /* Main Window display */
    w.show();
    auto res = app.exec();
    Core::instance()->settings()->save();
    return res;
}
#endif
/**
 *  VLMC Entry point
 *  \brief this is the VLMC entry point
 *  \param argc
 *  \param argv
 *  \return Return value of vlmc
 */
int
VLMCCoremain( int argc, char **argv )
{
    QCoreApplication app( argc, argv );

    Backend::IBackend* backend;
    VLMCmainCommon( app, &backend );
    auto coreLock = Core::Policy_t::lock();

    /* Load a project file */
    if ( app.arguments().count() < 3 )
    {
        vlmcCritical() << "Usage: ./vlmc project.vlmc output_file";
        return 1;
    }


#ifndef HAVE_GUI
    ConsoleRenderer renderer;
    Project  *p = Core::instance()->project();

    QCoreApplication::connect( p, &Project::projectLoaded, &renderer, &ConsoleRenderer::startRender );
    p->load( app.arguments()[1] );
#endif
    auto res = app.exec();
    Core::instance()->settings()->save();
    return res;
}

int
VLMCmain( int argc, char **argv )
{
#ifdef HAVE_GUI
    int res = VLMCGuimain( argc, argv );
#else
    int res = VLMCCoremain( argc, argv );
#endif
    return res;
}

