/*****************************************************************************
 * PreferenceWidget.h: Abstract class that will be used to save load / preferences
 * values.
 *****************************************************************************
 * Copyright (C) 2008-2016 VideoLAN
 *
 * Authors: Clement CHAVANCE <kinder@vlmc.org>
 *          Hugo Beauzée-Luyssen <hugo@beauzee.fr>
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

#ifndef PREFERENCEWIDGET_H
#define PREFERENCEWIDGET_H

#include <QScrollArea>
#include <QString>
#include <QHash>
#include "Settings/Settings.h"

class   ISettingsCategoryWidget;
class   SettingValue;
class   QLabel;
class   QEvent;

class   PreferenceWidget : public QScrollArea
{
    Q_OBJECT
    public:
        typedef QList<ISettingsCategoryWidget*>    SettingsList;
        PreferenceWidget(const QString &name, const char* category, Settings *settings, QWidget* parent = 0 );
        virtual ~PreferenceWidget() {}

        virtual bool    save();
        virtual void    reset();
        void            discard();
        const char      *category() const;
    protected:
        void            changeEvent( QEvent *e );

    private:
        ISettingsCategoryWidget        *widgetFactory( SettingValue* s );
        void            retranslateUi();

    private:
        const char                  *m_category;
        SettingsList                m_settings;
        QHash<SettingValue*, QLabel*>  m_labels;
};

#endif
