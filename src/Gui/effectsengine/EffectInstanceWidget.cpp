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

EffectInstanceWidget::EffectInstanceWidget( EffectInstance *effect, QWidget *parent ) :
    QDialog( parent ),
    m_ui( new Ui::EffectSettingWidget )
{
    m_ui->setupUi( this );
    m_ui->nameValueLabel->setText( effect->effect()->name() );
    m_ui->descValueLabel->setText( effect->effect()->description() );
    m_ui->typeValueLabel->setText( nameFromType( effect->effect()->type() ) );
    m_ui->authorValueLabel->setText( effect->effect()->author() );
    QString version = QString::number( effect->effect()->getMajor() ) + '.' +
                      QString::number( effect->effect()->getMinor() );
    m_ui->versionValueLabel->setText( version );
    EffectInstance::ParamList::iterator         it = effect->params().begin();
    EffectInstance::ParamList::iterator         ite = effect->params().end();
    while ( it != ite )
    {
        EffectSettingValue          *s = it.value();
        ISettingsCategoryWidget     *widget = widgetFactory( s );
        QLabel                      *label = new QLabel( tr( s->name() ), this );
        widget->widget()->setToolTip( s->description() );
        m_ui->settingsLayout->addRow( label , widget->widget() );
        m_settings.push_back( widget );
        ++it;
    }
    connect( m_ui->buttons, SIGNAL( clicked( QAbstractButton* ) ),
             this, SLOT( buttonsClicked( QAbstractButton* ) ) );
}

QString
EffectInstanceWidget::nameFromType( Effect::Type type )
{
    switch ( type )
    {
    case Effect::Filter:
        return tr( "Filter" );
    case Effect::Source:
        return tr( "Source" );
    case Effect::Mixer2:
        return tr( "Mixer 2" );
    case Effect::Mixer3:
        return tr( "Mixer 3" );
    default:
        return tr( "Unknown type" );
    }
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

void
EffectInstanceWidget::buttonsClicked( QAbstractButton *button )
{
    switch ( m_ui->buttons->standardButton( button ) )
    {
    case QDialogButtonBox::Ok:
    case QDialogButtonBox::Apply:
        save();
    default:
        break ;
    }
}
