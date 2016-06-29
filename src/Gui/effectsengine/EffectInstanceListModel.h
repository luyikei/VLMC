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

#ifndef EFFECTINSTANCELISTMODEL_H
#define EFFECTINSTANCELISTMODEL_H

namespace Backend
{
class   IInput;
}

#include <QAbstractListModel>

class EffectHelper;

class EffectInstanceListModel : public QAbstractListModel
{
    public:
        EffectInstanceListModel( Backend::IInput *input );
        virtual qint32      rowCount( const QModelIndex &parent ) const;
        virtual QVariant    data( const QModelIndex &index, int role ) const;
        virtual bool        removeRows( int row, int count, const QModelIndex &parent = QModelIndex() );
        void                moveUp( const QModelIndex &index );
        void                moveDown( const QModelIndex &index );
        EffectHelper*       add( const QString &effectName );

    private:
        Backend::IInput*  m_input;
};

#endif // EFFECTINSTANCELISTMODEL_H
