/*****************************************************************************
 * EffectsListView.h: Display a list of effects
 *****************************************************************************
 * Copyright (C) 2008-2010 VideoLAN
 *
 * Authors: Hugo Beauzee-Luyssen <beauze.h@gmail.com>
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

#ifndef EFFECTLISTVIEW_H
#define EFFECTLISTVIEW_H

#include "EffectsEngine/Effect.h"

#include <QListView>

class   QStandardItemModel;

class EffectsListView : public QListView
{
    Q_OBJECT

    public:
        explicit            EffectsListView(QWidget *parent = 0);
        void                setType( Effect::Type type );

    protected:
        void                mousePressEvent( QMouseEvent *event );
        void                mouseMoveEvent( QMouseEvent *event );

    private:
        QStandardItemModel  *m_model;
        Effect::Type        m_type;
        QPoint              m_dragStartPos;
    public slots:
        void                effectAdded( Effect *effect, const QString& name,
                                         Effect::Type type );
    private slots:
        void                effectActivated( const QModelIndex& index ) const;
};

#endif // EFFECTLISTVIEW_H
