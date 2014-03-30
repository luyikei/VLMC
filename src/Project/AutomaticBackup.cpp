/*****************************************************************************
 * AutomaticBackup.cpp: Handles the project automatic backup & associated settings
 *****************************************************************************
 * Copyright (C) 2008-2014 VideoLAN
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

#include <QTimer>

#include "AutomaticBackup.h"
#include "Project.h"
#include "Settings/Settings.h"

AutomaticBackup::AutomaticBackup( Settings* vlmcSettings, QObject *parent )
    : QObject(parent)
    , m_vlmcSettings( vlmcSettings )
{
    m_timer = new QTimer( this );
    m_vlmcSettings->createVar( SettingValue::Bool, "vlmc/AutomaticBackup", false,
                                 QT_TRANSLATE_NOOP( "PreferenceWidget", "Automatic save" ),
                                 QT_TRANSLATE_NOOP( "PreferenceWidget", "When this option is activated,"
                                             "VLMC will automatically save your project "
                                             "at a specified interval" ), SettingValue::Nothing );
    m_vlmcSettings->createVar( SettingValue::Int, "vlmc/AutomaticBackupInterval", 5,
                                QT_TRANSLATE_NOOP( "PreferenceWidget", "Automatic save interval" ),
                                QT_TRANSLATE_NOOP( "PreferenceWidget", "This is the interval that VLMC will wait "
                                            "between two automatic save" ), SettingValue::Nothing );

    vlmcSettings->watchValue( "vlmc/AutomaticBackup", this, SLOT( automaticSaveEnabledChanged( QVariant ) ) );
    vlmcSettings->watchValue( "vlmc/AutomaticBackupInterval", this, SLOT( automaticSaveIntervalChanged( QVariant ) ) );
}

AutomaticBackup::~AutomaticBackup()
{
    delete m_timer;
}

void
AutomaticBackup::setProject( Project* projectManager )
{
    m_timer->disconnect();
    connect( m_timer, SIGNAL( timeout() ), projectManager, SLOT(autoSaveRequired() ) );
}

void
AutomaticBackup::automaticSaveEnabledChanged( const QVariant& val )
{
    bool    enabled = val.toBool();

    if ( enabled == true )
    {
        int interval = m_vlmcSettings->value( "vlmc/AutomaticBackupInterval" )->get().toInt();
        m_timer->start( interval * 1000 * 60 );
    }
    else
        m_timer->stop();
}

void
AutomaticBackup::automaticSaveIntervalChanged( const QVariant& val )
{
    bool enabled = m_vlmcSettings->value( "vlmc/AutomaticBackup" )->get().toBool();

    if ( enabled == false )
        return ;
    m_timer->start( val.toInt() * 1000 * 60 );
}
