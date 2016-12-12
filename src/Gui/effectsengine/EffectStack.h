/*****************************************************************************
 * EffectStack.h: Represent an effect stack, and allow parameters editing.
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

#ifndef EFFECTSTACK_H
#define EFFECTSTACK_H

#include <QDialog>
#include <QStandardItemModel>

namespace Backend
{
class IInput;
class IFilter;
}

class   EffectHelper;
class   EffectInstanceListModel;
class   EffectInstanceWidget;

class   QStackedLayout;

namespace Ui
{
    class EffectStack;
}

class EffectStack : public QDialog
{
    Q_OBJECT

    public:
        explicit EffectStack( Backend::IInput* input, QWidget *parent = 0 );
        ~EffectStack();

    private:
        void        addEffectHelper( EffectHelper* helper );

    private slots:
        void        selectedChanged( const QModelIndex &index );
        void        moveUp();
        void        moveDown();
        void        remove();
        void        add();

    private:
        Ui::EffectStack                 *m_ui;
        EffectInstanceListModel         *m_model;
        QStackedLayout                  *m_stackedLayout;
        QHash<QString, EffectInstanceWidget*>   m_instanceWidgets;
};

#endif // EFFECTSTACK_H
