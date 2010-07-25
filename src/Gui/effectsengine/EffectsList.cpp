/*****************************************************************************
 * EffectsList.cpp: List the available effects plugin
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

#include "EffectsList.h"
#include "EffectsEngine/EffectsEngine.h"
#include "ui_EffectsList.h"

#include <QStandardItemModel>

EffectsList::EffectsList(QWidget *parent) :
    QWidget(parent),
    m_ui( new Ui::EffectsList )
{
    m_ui->setupUi( this );
    m_filtersModel = new QStandardItemModel( this );
    m_effectsModel = new QStandardItemModel( this );

    m_ui->filterList->setModel( m_filtersModel );
    m_ui->effectsList->setModel( m_effectsModel );
    connect( EffectsEngine::getInstance(), SIGNAL( effectAdded( Effect*, Effect::Type ) ),
             this, SLOT( effectAdded(Effect*,Effect::Type) ) );
}

EffectsList::~EffectsList()
{
    delete m_ui;
}

void
EffectsList::effectAdded( Effect *effect, Effect::Type type )
{
    if ( type == Effect::Filter )
        m_filtersModel->appendRow( new QStandardItem( effect->name() ) );
    else
        m_effectsModel->appendRow( new QStandardItem( effect->name() ) );
}
