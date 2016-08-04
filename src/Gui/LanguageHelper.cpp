/*****************************************************************************
 * Languagehelper.cpp: Watch for language changes
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

#include "LanguageHelper.h"

#include <QApplication>
#include <QLibraryInfo>
#include <QLocale>
#include <QTranslator>
#include <QVariant>

#define TS_PREFIX "vlmc_"

LanguageHelper::LanguageHelper() : m_translator( nullptr ), m_qtTranslator( nullptr )
{
    connect( qApp, SIGNAL( aboutToQuit() ), this, SLOT( deleteLater() ) );
}

LanguageHelper::~LanguageHelper()
{
    if ( m_translator )
        delete m_translator;
    if ( m_qtTranslator )
        delete m_qtTranslator;
}

void
LanguageHelper::languageChanged( const QVariant &vLang )
{
    languageChanged( vLang.toString() );
}

void
LanguageHelper::languageChanged( const QString &lang  )
{
    if ( m_translator != nullptr ||  m_qtTranslator != nullptr )
    {
        qApp->removeTranslator( m_translator );
        qApp->removeTranslator( m_qtTranslator );
        delete m_translator;
        delete m_qtTranslator;
        m_translator = nullptr;
        m_qtTranslator = nullptr;
    }

    m_translator = new QTranslator();
    m_qtTranslator = new QTranslator();

    if ( lang.isEmpty() || lang == "default" )
    {
        m_translator->load( TS_PREFIX + QLocale::system().name(), ":/ts/" );
        m_qtTranslator->load( "qt_" + QLocale::system().name(),
#if defined( Q_OS_WIN32 ) || defined( Q_OS_MAC )
                              qApp->applicationDirPath() + "/ts/" );
#else
                              QLibraryInfo::location( QLibraryInfo::TranslationsPath ) );
#endif
    }
    else
    {
        m_translator->load( TS_PREFIX + lang, ":/ts/" );
        m_qtTranslator->load( "qt_" + lang,
#if defined( Q_OS_WIN32 ) || defined( Q_OS_MAC )
                              qApp->applicationDirPath() + "/ts/" );
#else
                              QLibraryInfo::location( QLibraryInfo::TranslationsPath ) );
#endif
    }

    qApp->installTranslator( m_translator );   // For translating VLMC UI strings
    qApp->installTranslator( m_qtTranslator ); // For translating Qt's dialog buttons etc.
}
