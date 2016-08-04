/*****************************************************************************
 * StringWidget.h: Handle text settings.
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

#include "StringWidget.h"
#include "Settings/SettingValue.h"

#include <QLineEdit>

StringWidget::StringWidget( SettingValue *s, QWidget *parent /*= nullptr*/ ) :
        ISettingsCategoryWidget( parent, s )
{
    m_lineEdit = new QLineEdit( this );
    if ( ( s->flags() & SettingValue::Password ) != 0 )
        m_lineEdit->setEchoMode( QLineEdit::Password );
    layout()->addWidget( m_lineEdit );
    changed( s->get() );
}

bool
StringWidget::save()
{
    if ( ( m_setting->flags() & SettingValue::NotEmpty ) != 0 )
    {
        if ( m_lineEdit->text().length() == 0 )
            return false;
    }
    m_setting->set( m_lineEdit->text() );
    return true;
}
void
StringWidget::changed( const QVariant &val )
{
    m_lineEdit->setText( val.toString() );
}
