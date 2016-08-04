/*****************************************************************************
 * EffectInstanceWidget.h: Display the settings for an EffectInstance
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

#include "EffectInstanceWidget.h"

#include "Gui/settings/BoolWidget.h"
#include "Gui/settings/IntWidget.h"
#include "Gui/settings/StringWidget.h"
#include "Gui/settings/DoubleWidget.h"
#include "Gui/settings/DoubleSliderWidget.h"
#include "EffectsEngine/EffectHelper.h"
#include "Backend/IFilter.h"
#include "Main/Core.h"
#include "Tools/VlmcDebug.h"

#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>

EffectInstanceWidget::EffectInstanceWidget( QWidget *parent ) :
    QWidget( parent ),
    m_ui( new Ui::EffectSettingWidget ),
    m_helper( nullptr )
{
    m_ui->setupUi( this );
    clear();
    connect( m_ui->applyButton, SIGNAL( clicked() ),
             this, SLOT( save() ) );
}

void
EffectInstanceWidget::setEffectHelper( std::shared_ptr<EffectHelper> const& helper )
{
    clear();
    m_helper = helper;
    m_ui->effectWidget->setFilterInfo( helper->filterInfo() );

    for ( auto param : helper->filterInfo()->paramInfos() )
    {
        SettingValue*               s = helper->value( QString::fromStdString( param->identifier() ) );
        ISettingsCategoryWidget*    widget = widgetFactory( s );
        QLabel*                     label = new QLabel( tr( s->name() ), this );
        m_widgets.push_back( label );
        m_widgets.push_back( widget );
        widget->setToolTip( s->description() );
        m_ui->settingsLayout->addRow( label , widget );
        m_settings.push_back( widget );
    }
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
EffectInstanceWidget::widgetFactory( SettingValue *s )
{
    switch ( s->type() )
    {
    case    SettingValue::Bool:
        return new BoolWidget( s, this );
    case    SettingValue::Double:
        if ( s->flags().testFlag( SettingValue::Clamped ) == true )
            return new DoubleSliderWidget( s, this );
        else
            return new DoubleWidget( s, this );
    case    SettingValue::Int:
        return new IntWidget( s, this );
    case    SettingValue::String:
        return new StringWidget( s, this );
    default:
        return nullptr;
    }
}

void
EffectInstanceWidget::save()
{
    for ( ISettingsCategoryWidget* val : m_settings )
        val->save();
}
