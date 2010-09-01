/*****************************************************************************
 * EffectStack.cpp: Represent an effect stack, and allow parameters editing.
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

#include "EffectStack.h"
#include "ui_EffectStack.h"

#include "EffectInstanceListModel.h"

EffectStack::EffectStack( EffectUser *user, QWidget *parent ):
    QDialog( parent ),
    m_ui( new Ui::EffectStack ),
    m_user( user )
{
    m_ui->setupUi( this );

    m_model = new EffectInstanceListModel( user );
    m_ui->list->setModel( m_model );
    connect( m_ui->list, SIGNAL( clicked( QModelIndex ) ),
             this, SLOT( selectedChanged( QModelIndex ) ) );
    connect( m_ui->upButton, SIGNAL( clicked() ), this, SLOT( moveUp() ) );
    connect( m_ui->downButton, SIGNAL( clicked() ), this, SLOT( moveDown() ) );
    connect( m_ui->removeButton, SIGNAL( clicked() ), this, SLOT( remove() ) );
}

EffectStack::~EffectStack()
{
    delete m_model;
    delete m_ui;
}

void
EffectStack::selectedChanged( const QModelIndex &index )
{
    m_ui->instanceWidget->setEffectInstance( m_model->data( index, Qt::EditRole ).value<EffectsEngine::EffectHelper*>()->effect );
}

void
EffectStack::moveUp()
{
    m_model->moveUp( m_ui->list->currentIndex() );
    if ( m_ui->list->currentIndex().row() > 0 )
        m_ui->list->setCurrentIndex( m_ui->list->currentIndex().sibling( m_ui->list->currentIndex().row() - 1, 0 ) );
}

void
EffectStack::moveDown()
{
    m_model->moveDown( m_ui->list->currentIndex() );
    if ( m_ui->list->currentIndex().row() < m_model->rowCount( QModelIndex() ) - 1 )
        m_ui->list->setCurrentIndex( m_ui->list->currentIndex().sibling( m_ui->list->currentIndex().row() + 1, 0 ) );
}

void
EffectStack::remove()
{
    m_model->removeRow( m_ui->list->currentIndex().row() );
}
