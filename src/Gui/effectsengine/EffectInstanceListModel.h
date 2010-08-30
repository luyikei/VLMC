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

#ifndef EFFECTINSTANCELISTMODEL_H
#define EFFECTINSTANCELISTMODEL_H

#include "EffectsEngine.h"

#include <QAbstractListModel>

class EffectInstanceListModel : public QAbstractListModel
{
    public:
        EffectInstanceListModel( EffectsEngine::EffectList *list );
        virtual qint32      rowCount( const QModelIndex &parent ) const;
        virtual QVariant    data( const QModelIndex &index, int role ) const;
        void                moveUp( const QModelIndex &index );
        void                moveDown( const QModelIndex &index );

    private:
        EffectsEngine::EffectList   *m_list;
};

#endif // EFFECTINSTANCELISTMODEL_H
