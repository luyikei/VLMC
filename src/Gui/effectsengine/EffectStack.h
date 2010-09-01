/*****************************************************************************
 * EffectStack.h: Represent an effect stack, and allow parameters editing.
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

#ifndef EFFECTSTACK_H
#define EFFECTSTACK_H

#include <QDialog>

class   EffectUser;

#include <QStandardItemModel>

class   EffectInstanceListModel;

namespace Ui
{
    class EffectStack;
}

class EffectStack : public QDialog
{
    Q_OBJECT

    public:
        explicit EffectStack( EffectUser *user, QWidget *parent = 0 );
        ~EffectStack();

    private slots:
        void        selectedChanged( const QModelIndex &index );
        void        moveUp();
        void        moveDown();
        void        remove();

    private:
        Ui::EffectStack                 *m_ui;
        EffectInstanceListModel         *m_model;
        EffectUser                      *m_user;
};

#endif // EFFECTSTACK_H
