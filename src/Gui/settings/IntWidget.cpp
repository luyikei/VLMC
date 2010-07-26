/*****************************************************************************
 * IntWidget.cpp Handle integer settings.
 *****************************************************************************
 * Copyright (C) 2008-2010 VideoLAN
 *
 * Authors: Hugo Beauzée-Luyssen <hugo@vlmc.org>
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

#include "IntWidget.h"
#include "SettingValue.h"

#include <QSpinBox>

IntWidget::IntWidget( SettingValue *s, QWidget *parent /*= NULL*/ ) :
        m_setting( s )
{
    m_spinbox = new QSpinBox( parent );
    connect( s, SIGNAL( changed( const QVariant& ) ),
             this, SLOT( changed( const QVariant& ) ) );
    if ( ( s->flags() & SettingValue::Clamped ) != 0 )
    {
        if ( s->min().isValid() )
            m_spinbox->setMinimum( s->min().toInt() );
        if ( s->max().isValid() )
            m_spinbox->setMaximum( s->max().toInt() );
    }
    if ( ( s->flags() & SettingValue::EightMultiple ) != 0 )
    {
        m_spinbox->setSingleStep( 8 );
    }
    changed( s->get() );
}

QWidget*
IntWidget::widget()
{
    return m_spinbox;
}

void
IntWidget::save()
{
    m_setting->set( m_spinbox->value() );
}

void
IntWidget::changed( const QVariant &val )
{
    m_spinbox->setValue( val.toInt() );
}
