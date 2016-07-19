/*****************************************************************************
 * EffectWidget.cpp: Display info about an effect.
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

#include "Backend/IFilter.h"
#include "EffectWidget.h"
#include "ui/EffectWidget.h"

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
EffectWidget::setFilterInfo( Backend::IFilterInfo* info )
{
    clear();
    m_ui->nameValueLabel->setText( QString::fromStdString( info->name() ) );
    m_ui->descValueLabel->setText( QString::fromStdString( info->description() ) );
    m_ui->authorValueLabel->setText( QString::fromStdString( info->author() ) );

}

void
EffectWidget::clear()
{
    m_ui->nameValueLabel->setText( QStringLiteral( "" ) );
    m_ui->descValueLabel->setText( QStringLiteral( "" ) );
    m_ui->typeValueLabel->setText( QStringLiteral( "" ) );
    m_ui->authorValueLabel->setText( QStringLiteral( "" ) );
    m_ui->versionValueLabel->setText( QStringLiteral( "" ) );
}
