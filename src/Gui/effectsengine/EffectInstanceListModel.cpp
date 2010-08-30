/*****************************************************************************
 * EffectInstanceListModel.cpp: Handle the model part of displaying an effect instance list.
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

#include "EffectInstanceListModel.h"
#include "EffectInstance.h"

#include <QApplication>
#include <QFontMetrics>

#include <QtDebug>

EffectInstanceListModel::EffectInstanceListModel( EffectsEngine::EffectList *list ) :
        m_list( list )
{
}

qint32
EffectInstanceListModel::rowCount( const QModelIndex& ) const
{
    return m_list->size();
}

QVariant
EffectInstanceListModel::data( const QModelIndex &index, int role ) const
{
    switch ( role )
    {
    case Qt::DisplayRole:
        return m_list->at( index.row() )->effect->effect()->name();
    case Qt::ToolTipRole:
        return m_list->at( index.row() )->effect->effect()->description();
    case Qt::EditRole:
        return QVariant::fromValue( m_list->at( index.row() ) );
    case Qt::SizeHintRole:
        {
            const QFontMetrics  &fm = QApplication::fontMetrics();
            QSize               size( fm.width( m_list->at( index.row() )->effect->effect()->name() ), fm.height() );
            return size;
        }
    default:
        return QVariant();
    }
}

bool
EffectInstanceListModel::removeRows( int row, int count, const QModelIndex& index /*= QModelIndex()*/ )
{
    if ( count != 1 )
        return false;
    if ( row < 0 || row >= m_list->size() )
        return false;
    beginRemoveRows( index, row, row );
    m_list->removeAt( row );
    endRemoveRows();
    return true;
}

void
EffectInstanceListModel::moveUp( const QModelIndex &index )
{
    if ( index.row() == 0 || index.row() >= m_list->size() )
        return ;
    emit layoutAboutToBeChanged();
    m_list->swap( index.row(), index.row() - 1 );
    emit layoutChanged();
}

void
EffectInstanceListModel::moveDown( const QModelIndex &index )
{
    if ( index.row() >= m_list->size() - 1 )
        return ;
    emit layoutAboutToBeChanged();
    m_list->swap( index.row(), index.row() + 1 );
    emit layoutChanged();
}
