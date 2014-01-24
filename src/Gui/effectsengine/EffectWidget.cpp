/*****************************************************************************
 * EffectWidget.cpp: Display info about an effect.
 *****************************************************************************
 * Copyright (C) 2008-2010 VideoLAN
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

#include "EffectWidget.h"
#include "ui_EffectWidget.h"

EffectWidget::EffectWidget(QWidget *parent) :
    QWidget( parent ),
    m_ui( new Ui::EffectWidget )
{
    m_ui->setupUi(this);
}

EffectWidget::~EffectWidget()
{
    delete m_ui;
}

void
EffectWidget::setEffect( Effect *effect )
{
    clear();
    m_effect = effect;
    m_ui->nameValueLabel->setText( m_effect->name() );
    m_ui->descValueLabel->setText( m_effect->description() );
    m_ui->typeValueLabel->setText( nameFromType( m_effect->type() ) );
    m_ui->authorValueLabel->setText( m_effect->author() );
    QString version = QString::number( m_effect->getMajor() ) + '.' +
                      QString::number( m_effect->getMinor() );
    m_ui->versionValueLabel->setText( version );

}

void
EffectWidget::clear()
{
    m_ui->nameValueLabel->setText( "" );
    m_ui->descValueLabel->setText( "" );
    m_ui->typeValueLabel->setText( "" );
    m_ui->authorValueLabel->setText( "" );
    m_ui->versionValueLabel->setText( "" );
}

QString
EffectWidget::nameFromType( Effect::Type type )
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
