/*****************************************************************************
 * EffectInstanceWidget.h: Display the settings for an EffectInstance
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

#ifndef EFFECTINSTANCEWIDGET_H
#define EFFECTINSTANCEWIDGET_H

#include <QWidget>
#include <memory>

#include "ui/EffectInstanceWidget.h"

namespace Backend
{
class IFilter;
}

class   SettingValue;
class   EffectHelper;
class   ISettingsCategoryWidget;

class EffectInstanceWidget : public QWidget
{
    Q_OBJECT

    public:
        explicit EffectInstanceWidget( QWidget *parent = 0);
        void     setEffectHelper( std::shared_ptr<EffectHelper> const& filter );
    private:
        ISettingsCategoryWidget*            widgetFactory( SettingValue *s );
        void                                clear();
    private:
        QList<ISettingsCategoryWidget*>     m_settings;
        QList<QWidget*>                     m_widgets;
        Ui::EffectSettingWidget             *m_ui;
        std::shared_ptr<EffectHelper>       m_helper;

    public slots:
        void                                save();
};

#endif // EFFECTINSTANCEWIDGET_H
