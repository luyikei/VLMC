/*****************************************************************************
 * LanguageWidget.h: Handle languge settings
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

#include "LanguageWidget.h"
#include "Settings/Settings.h"
#include "Settings/SettingValue.h"
#include "Tools/VlmcDebug.h"

#include <QApplication>
#include <QComboBox>
#include <QDir>
#include <QTranslator>

#define TS_PREFIX "vlmc_"

LanguageWidget::LanguageWidget( SettingValue *s, QWidget *parent /*= nullptr*/ ) :
        ISettingsCategoryWidget( parent, s )
{
    m_list = new QComboBox( this );
    QDir dir( ":/ts/", "*.qm", QDir::Name | QDir::IgnoreCase, QDir::Files );

    foreach ( const QString& tsFileName, dir.entryList() )
    {
        QString     countryCode;
        int         localePos = tsFileName.lastIndexOf( TS_PREFIX );
        int         extPos = tsFileName.lastIndexOf( ".qm" );

        if ( localePos < 0 || extPos < 0 || localePos > extPos )
        {
            vlmcWarning() << "Invalid translation file:" << tsFileName;
            continue ;
        }

        localePos += qstrlen( TS_PREFIX );
        countryCode = tsFileName.mid( localePos, extPos - localePos );
        QLocale     locale( countryCode );

        // Check if the country code is valid ISO 639
        if ( locale.language() == QLocale::C )
        {
            vlmcWarning() << "Invalid locale for file:" << tsFileName;
            continue;
        }

        m_list->addItem( QString( "%1 (%2)" ).arg(
                QLocale::languageToString( locale.language() ),
                QLocale::countryToString( locale.country() ) ), countryCode );
    }
    // Add the built-in us_US locale.
    m_list->addItem( "English (UnitedStates)", "en_US" );

    // Sort the combobox
    m_list->model()->sort( 0 );

    // Add the system default option (auto-detection of the locale)
    m_list->insertItem( 0, "System Locale (autodetect)", "default" );
    layout()->addWidget( m_list );
    changed( s->get() );
}

bool
LanguageWidget::save()
{
    QString     lang = m_list->itemData( m_list->currentIndex() ).toString();
    m_setting->set( lang );
    return true;
}

void
LanguageWidget::changed( const QVariant &val )
{
    int idx = m_list->findData( val.toString() );
    if ( idx != -1 )
        m_list->setCurrentIndex( idx );
}
