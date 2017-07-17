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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

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
#include <QCommandLineParser>

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#endif

static void
setupCommandLine( QCommandLineParser& parser )
{
    parser.setSingleDashWordOptionMode( QCommandLineParser::ParseAsLongOptions );
    parser.setApplicationDescription(
                QString(
                    "VideoLAN Movie Creator (VLMC) is a cross-platform, non-linear\n"
                    "video editing software."
                    ) );
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addPositionalArgument( "project",
                                  QCoreApplication::translate( "main", "Project file to open." ),
                                  "[filename|URI]" );
    parser.addPositionalArgument( "output",
                                  QCoreApplication::translate( "main", "Output file to write to." ),
                                  "[filename]" );

    parser.addOption( { "v",
                        QCoreApplication::translate( "main", "Log level - Verbose" ) } );
    parser.addOption( { "vv",
                        QCoreApplication::translate( "main", "Log level - Debug" ) } );
    parser.addOption( { { "b", "backendverbose" },
                        QCoreApplication::translate( "main", "Backend Log level to set" ),
                        "value" } );
    parser.process( *qApp );
}


static void
VLMCmainCommon( Backend::IBackend** backend )
{
    qRegisterMetaType<Workflow::TrackType>( "Workflow::TrackType" );
    qRegisterMetaType<Vlmc::FrameChangedReason>( "Vlmc::FrameChangedReason" );
    qRegisterMetaType<QVariant>( "QVariant" );
    qRegisterMetaType<QUuid>( "QUuid" );

    *backend = Backend::instance();
}

/**
 *  VLMC Entry point
 *  \brief this is the VLMC entry point
 *  \return Return value of vlmc
 */
#ifdef HAVE_GUI
int
VLMCGuimain( const QString& projectFile )
{
#ifdef Q_WS_X11
    XInitThreads();
#endif

    Backend::IBackend* backend;
    VLMCmainCommon( &backend );

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
    if ( projectFile.isEmpty() == true )
        w.showWizard();

    /* Main Window display */
    w.show();

    if ( projectFile.isEmpty() == false )
        Core::instance()->loadProject( projectFile );

    auto res = qApp->exec();
    Core::instance()->settings()->save();
    return res;
}
#endif

/**
 *  VLMC Entry point
 *  \brief this is the VLMC entry point
 *  \return Return value of vlmc
 */
int
VLMCCoremain( const QString& projectFile , const QString& outputFile)
{
    Backend::IBackend* backend;
    VLMCmainCommon( &backend );

    ConsoleRenderer renderer( outputFile );
    Project  *p = Core::instance()->project();

    QCoreApplication::connect( p, &Project::projectLoaded, &renderer, &ConsoleRenderer::startRender );
    QCoreApplication::connect( &renderer, &ConsoleRenderer::finished, qApp, &QCoreApplication::quit, Qt::QueuedConnection );
    Core::instance()->settings()->load();
    p->load( projectFile );

    auto res = qApp->exec();
    Core::instance()->settings()->save();
    return res;
}

int
VLMCmain( int argc, char **argv )
{
#ifdef HAVE_GUI
    QApplication app( argc, argv );
#else
    QCoreApplication app( argc, argv );
#endif

    app.setApplicationName( "VLMC" );
    app.setOrganizationName( "VideoLAN" );
    app.setOrganizationDomain( "videolan.org" );
    app.setApplicationVersion(
                QString(
                    "%1 '%2'\n"
                    "VideoLAN Movie Creator (VLMC) is a cross-platform, non-linear\n"
                    "video editing software.\n"
                    "Copyright (C) 2008-10 VideoLAN\n"
                    "This is free software; see the source for copying conditions. There is NO\n"
                    "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"
                    ).arg( PACKAGE_VERSION ).arg( CODENAME ) );

    QCommandLineParser parser;
    setupCommandLine( parser );

    const auto& args = parser.positionalArguments();

    if ( args.size() >= 2  )
        return VLMCCoremain( args.at( 0 ), args.at( 1 ) );
#ifdef HAVE_GUI
    else if ( args.size() == 1 )
        return VLMCGuimain( args.at( 0 ) );
    else
        return VLMCGuimain( "" );
#else
    else
        parser.showHelp( 1 ); // This function exits the application. No need to return any value.
#endif
}

