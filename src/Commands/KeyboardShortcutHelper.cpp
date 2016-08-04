/*****************************************************************************
 * KeyboardShortcutHelper.cpp: An helper to catch keyboard shortcut modifications
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

#include <QAction>

#include "KeyboardShortcutHelper.h"

#include "Settings/Settings.h"

KeyboardShortcutHelper::KeyboardShortcutHelper( const QString& name, QWidget* parent ) :
        QShortcut( parent ),
        m_name( name ),
        m_action( nullptr )
{
    SettingValue* setting = Core::instance()->settings()->value( name );
    setKey( QKeySequence( setting->get().toString() ) );
    connect( setting, &SettingValue::changed, this, &KeyboardShortcutHelper::shortcutUpdated );
}

KeyboardShortcutHelper::KeyboardShortcutHelper( const QString& name, QAction *action,
                                                QWidget *parent /*= nullptr*/ ) :
    QShortcut( parent ),
    m_name( name ),
    m_action( action )
{
    SettingValue* setting = Core::instance()->settings()->value( name );
    action->setShortcut( setting->get().toString() );
    connect( setting, &SettingValue::changed, this, &KeyboardShortcutHelper::shortcutUpdated );
}

void
KeyboardShortcutHelper::shortcutUpdated( const QVariant& value )
{
    if ( m_action == nullptr )
        setKey( QKeySequence( value.toString() ) );
    else
        m_action->setShortcut( value.toString() );
}
