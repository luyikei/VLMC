/*****************************************************************************
 * ColorWidget.cpp: Handle Settings of type Color
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

#ifndef COLORWIDGET_H
#define COLORWIDGET_H

#include "ISettingsCategoryWidget.h"
#include <QColor>

class   SettingValue;

class   QPushButton;

class   ColorWidget : public ISettingsCategoryWidget
{
    Q_OBJECT

    public:
        ColorWidget( SettingValue *s, QWidget *parent = nullptr );
        bool                    save();

    private slots:
        virtual void            changed( const QVariant& );
        void                    buttonClicked();

    private:
        QPushButton             *m_button;
        QColor                  m_color;

};

#endif // COLORWIDGET_H
