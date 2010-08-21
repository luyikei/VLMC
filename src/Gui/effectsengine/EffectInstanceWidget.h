/*****************************************************************************
 * EffectInstanceWidget.h: Display the settings for an EffectInstance
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

#ifndef EFFECTINSTANCEWIDGET_H
#define EFFECTINSTANCEWIDGET_H

#include <QDialog>

class   EffectInstance;
class   EffectSettingValue;

#include "Effect.h"
#include "ui_EffectInstanceWidget.h"

class   ISettingsCategoryWidget;

class EffectInstanceWidget : public QDialog
{
    Q_OBJECT

    public:
        explicit EffectInstanceWidget( EffectInstance* effect, QWidget *parent = 0);

    private:
        static QString                      nameFromType( Effect::Type type );
        ISettingsCategoryWidget             *widgetFactory( EffectSettingValue *s );
        void                                save();
    private:
        EffectInstance                      *m_effect;
        QList<ISettingsCategoryWidget*>     m_settings;
        Ui::EffectSettingWidget             *m_ui;

    private slots:
        void        buttonsClicked( QAbstractButton* button );
};

#endif // EFFECTINSTANCEWIDGET_H
