/*****************************************************************************
 * EffectInstanceListModel.cpp: Handle the model part of displaying an effect instance list.
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

#include "EffectInstanceListModel.h"

#include "Main/Core.h"
#include "EffectsEngine/EffectHelper.h"

#include "Backend/MLT/MLTFilter.h"
#include "Backend/IInput.h"
#include "Backend/IBackend.h"

#include <QApplication>
#include <QFontMetrics>

EffectInstanceListModel::EffectInstanceListModel( Backend::IInput* input ) :
        m_input( input )
{
}

qint32
EffectInstanceListModel::rowCount( const QModelIndex& ) const
{
    return m_input->filterCount();
}

QVariant
EffectInstanceListModel::data( const QModelIndex &index, int role ) const
{
    auto filter         = m_input->filter( index.row() );
    auto id             = QString::fromStdString( filter->identifier() );
    auto info           = Backend::instance()->filterInfo( filter->identifier() );

    switch ( role )
    {
    case Qt::DisplayRole:
        return QString::fromStdString( info->name() );
    case Qt::ToolTipRole:
        return QString::fromStdString( info->description() );
    case Qt::EditRole:
        return id;
    case Qt::SizeHintRole:
        {
            const QFontMetrics  &fm = QApplication::fontMetrics();
            QSize               size( fm.width( QString::fromStdString( info->name() ) ), fm.height() );
            return size;
        }
    default:
        return QVariant();
    }
}

bool
EffectInstanceListModel::removeRows( int row, int count, const QModelIndex& index /*= QModelIndex()*/ )
{
    if ( count != 1 || row < 0 || row >= m_input->filterCount() )
        return false;
    beginRemoveRows( index, row, row );
    m_input->detach( row );
    endRemoveRows();
    return true;
}

void
EffectInstanceListModel::moveUp( const QModelIndex &index )
{
    if ( index.row() <= 0 || index.row() >= m_input->filterCount() )
        return ;
    emit layoutAboutToBeChanged();
    m_input->moveFilter( index.row(), index.row() - 1 );
    emit layoutChanged();
}

void
EffectInstanceListModel::moveDown( const QModelIndex &index )
{
    if ( index.row() >= m_input->filterCount() )
        return ;
    emit layoutAboutToBeChanged();
    m_input->moveFilter( index.row(), index.row() + 1 );
    emit layoutChanged();
}

EffectHelper*
EffectInstanceListModel::add( const QString &effectName )
{
    if ( effectName.isEmpty() == true )
        return nullptr;
    beginInsertRows( QModelIndex(), m_input->filterCount(), m_input->filterCount() );
    EffectHelper* helper = nullptr;
    try
    {
        helper = new EffectHelper( effectName  );
    }
    catch( Backend::InvalidServiceException& e )
    {
        return nullptr;
    }
    m_input->attach( *helper->filter() );
    endInsertRows();
    return helper;
}
