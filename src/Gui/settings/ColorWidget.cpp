/*****************************************************************************
 * ColorWidget.cpp: Handle Settings of type Color
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

#include "ColorWidget.h"

#include "Settings/SettingValue.h"

#include <QColorDialog>
#include <QPushButton>

ColorWidget::ColorWidget( SettingValue *s, QWidget *parent ) :
        ISettingsCategoryWidget( parent, s )
{
    m_color = s->get().value<QColor>();
    m_button = new QPushButton( this );
    m_button->setPalette( QPalette(  m_color ) );
    layout()->addWidget( m_button );
    connect( m_button, SIGNAL( clicked() ), this, SLOT( buttonClicked() ) );
    changed( m_color );
}

bool
ColorWidget::save()
{
    m_setting->set( m_color );
    return true;
}

void
ColorWidget::changed( const QVariant &val )
{
    m_button->setPalette( QPalette( val.value<QColor>() ) );
}

void
ColorWidget::buttonClicked()
{
    QColor  color = QColorDialog::getColor( m_color, nullptr );
    if ( color.isValid() == true )
    {
        m_color = color;
        changed( color );
    }
}
