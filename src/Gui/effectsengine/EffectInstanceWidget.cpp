/*****************************************************************************
 * EffectInstanceWidget.h: Display the settings for an EffectInstance
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

#include "EffectInstanceWidget.h"

#include "BoolWidget.h"
#include "ColorWidget.h"
#include "DoubleSliderWidget.h"
#include "Effect.h"
#include "EffectInstance.h"
#include "EffectSettingValue.h"

#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>

EffectInstanceWidget::EffectInstanceWidget( QWidget *parent ) :
    QWidget( parent ),
    m_ui( new Ui::EffectSettingWidget )
{
    m_ui->setupUi( this );
    clear();
}

void
EffectInstanceWidget::setEffectInstance( EffectInstance *instance )
{
    clear();
    m_effect = instance;
    m_ui->effectWidget->setEffect( instance->effect() );

    EffectInstance::ParamList::iterator         it = instance->params().begin();
    EffectInstance::ParamList::iterator         ite = instance->params().end();
    while ( it != ite )
    {
        EffectSettingValue          *s = it.value();
        ISettingsCategoryWidget     *widget = widgetFactory( s );
        QLabel                      *label = new QLabel( tr( s->name() ), this );
        m_widgets.push_back( label );
        m_widgets.push_back( widget->widget() );
        widget->widget()->setToolTip( s->description() );
        m_ui->settingsLayout->addRow( label , widget->widget() );
        m_settings.push_back( widget );
        ++it;
    }
    connect( m_ui->applyButton, SIGNAL( clicked() ),
             this, SLOT( save() ) );
}

void
EffectInstanceWidget::clear()
{
    m_ui->effectWidget->clear();
    qDeleteAll( m_settings );
    m_settings.clear();
    qDeleteAll( m_widgets );
    m_widgets.clear();
}

ISettingsCategoryWidget*
EffectInstanceWidget::widgetFactory( EffectSettingValue *s )
{
    switch ( s->type() )
    {
    case    SettingValue::Bool:
        return new BoolWidget( s, this );
    case    SettingValue::Double:
        return new DoubleSliderWidget( s, this );
    case    SettingValue::Color:
        return new ColorWidget( s, this );
    default:
        return NULL;
    }
}

void
EffectInstanceWidget::save()
{
    foreach ( ISettingsCategoryWidget* val, m_settings )
        val->save();
}
