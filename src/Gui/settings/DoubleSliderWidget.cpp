/*****************************************************************************
 * DoubleSliderWidget.cpp Handle double settings using a slider for values.
 *****************************************************************************
 * Copyright (C) 2008-2010 VideoLAN
 *
 * Authors: Hugo Beauzée-Luyssen <beauze.h@gmail.com>
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

#include "DoubleSliderWidget.h"

#include "SettingValue.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>

DoubleSliderWidget::DoubleSliderWidget( SettingValue *s, QWidget *parent /*= NULL*/ ) :
        ISettingsCategoryWidget( s )
{
    m_container = new QWidget( parent );
    //Creating the slider
    m_slider = new QSlider( m_container );
    m_slider->setOrientation( Qt::Horizontal );
    //Creating the label
    m_valueDisplayer = new QLabel( QString::number( s->get().toDouble() ), m_container );
    //Avoid label resizing due to roundups
    const QFontMetrics  &fm = m_valueDisplayer->fontMetrics();
    m_valueDisplayer->setFixedWidth( fm.width( "0.00" ) );
    //Setting the layout:
    QHBoxLayout *layout = new QHBoxLayout( m_container );
    layout->addWidget( m_slider );
    layout->addWidget( m_valueDisplayer );
    //TODO: check if the value is clamped
    m_slider->setMaximum( s->max().toDouble() * 100.0 );
    m_slider->setMinimum( s->min().toDouble() * 100.0 );
    changed ( s->get() );
    connect( m_slider, SIGNAL( valueChanged( int ) ), this, SLOT( sliderMoved( int ) ) );
}

QWidget*
DoubleSliderWidget::widget()
{
    return m_container;
}

void
DoubleSliderWidget::save()
{
    m_setting->set( m_slider->value() / 100.0 );
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
