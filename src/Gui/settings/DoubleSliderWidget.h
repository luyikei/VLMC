/*****************************************************************************
 * DoubleSliderWidget.h Handle double settings using a slider for values.
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

#ifndef DOUBLESLIDERWIDGET_H
#define DOUBLESLIDERWIDGET_H

#include "ISettingsCategoryWidget.h"

class   SettingValue;

class   QSlider;
class   QLabel;

class DoubleSliderWidget : public ISettingsCategoryWidget
{
    Q_OBJECT

    public:
        DoubleSliderWidget( SettingValue *s, QWidget *parent = nullptr );
        bool        save();

    private:
        QSlider         *m_slider;
        QLabel          *m_valueDisplayer;

    private slots:
        virtual void    changed( const QVariant& );
        void            sliderMoved( int value );
};

#endif // DOUBLESLIDERWIDGET_H
