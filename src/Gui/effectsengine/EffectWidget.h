/*****************************************************************************
 * EffectWidget.h: Display info about an effect.
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

#ifndef EFFECTWIDGET_H
#define EFFECTWIDGET_H

#include <QWidget>

#include "EffectsEngine/Effect.h"

namespace Ui
{
    class EffectWidget;
}

class EffectWidget : public QWidget
{
    Q_OBJECT

    public:
        explicit EffectWidget( QWidget *parent = 0 );
        ~EffectWidget();
        void                setEffect( Effect* effect );
        void                clear();

    private:
        static QString                      nameFromType( Effect::Type type );

    private:
        Ui::EffectWidget    *m_ui;
        Effect              *m_effect;
};

#endif // EFFECTWIDGET_H
