/*****************************************************************************
 * DoubleSliderWidget.cpp Handle double settings using a slider for values.
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

#include "DoubleSliderWidget.h"

#include "Settings/SettingValue.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>

DoubleSliderWidget::DoubleSliderWidget( SettingValue *s, QWidget *parent /*= nullptr*/ ) :
        ISettingsCategoryWidget( parent, s )
{
    //Creating the slider
    m_slider = new QSlider( this );
    m_slider->setOrientation( Qt::Horizontal );
    //Creating the label
    m_valueDisplayer = new QLabel( QString::number( s->get().toDouble() ), this  );
    //Avoid label resizing due to roundups
    const QFontMetrics  &fm = m_valueDisplayer->fontMetrics();
    m_valueDisplayer->setFixedWidth( fm.width( "0.00" ) );
    //Setting the layout:
    layout()->addWidget( m_slider );
    layout()->addWidget( m_valueDisplayer );
    if ( s->flags().testFlag( SettingValue::Clamped ) == true )
    {
        if ( s->max().isValid() )
            m_slider->setMaximum( s->max().toDouble() * 100.0 );
        if ( s->min().isValid() )
            m_slider->setMinimum( s->min().toDouble() * 100.0 );
    }
    changed ( s->get() );
    connect( m_slider, SIGNAL( valueChanged( int ) ), this, SLOT( sliderMoved( int ) ) );
}

bool
DoubleSliderWidget::save()
{
    m_setting->set( m_slider->value() / 100.0 );
    return true;
}

void
DoubleSliderWidget::changed( const QVariant &val )
{
    m_slider->setValue( val.toDouble() * 100.0 );
}

void
DoubleSliderWidget::sliderMoved( int value )
{
    m_valueDisplayer->setText( QString::number( (double)value / 100.0 ) );
}
