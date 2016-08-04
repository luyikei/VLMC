/*****************************************************************************
 * KeyboardShortcut.h: Handle KeyboardShortcut setting type.
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

#include "KeyboardShortcut.h"
#include "KeyboardShortcutInput.h"
#include "Settings/SettingValue.h"

KeyboardShortcut::KeyboardShortcut( SettingValue *s, QWidget *parent /*= nullptr*/ ) :
        ISettingsCategoryWidget( parent, s )
{
    m_input = new KeyboardShortcutInput( s->name(), s->get().toString(), this );
    layout()->addWidget( m_input );
}

bool
KeyboardShortcut::save()
{
    m_setting->set( m_input->shortcut() );
    return true;
}
