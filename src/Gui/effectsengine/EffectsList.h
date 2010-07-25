/*****************************************************************************
 * EffectsList.h: List the available effects plugin
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

#ifndef EFFECTSLIST_H
#define EFFECTSLIST_H

#include <QWidget>
#include "EffectsEngine/Effect.h"

class   QStandardItemModel;

namespace Ui
{
    class EffectsList;
}

class EffectsList : public QWidget
{
    Q_OBJECT

    public:
        explicit EffectsList(QWidget *parent = 0);
        ~EffectsList();

    private:
        Ui::EffectsList     *m_ui;
        QStandardItemModel  *m_filtersModel;
        QStandardItemModel  *m_effectsModel;

    public slots:
        void                effectAdded( Effect *effect, Effect::Type type );
};

#endif // EFFECTSLIST_H
