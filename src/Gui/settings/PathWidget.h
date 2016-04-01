/*****************************************************************************
 * PathWidget: Handle path settings.
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

#ifndef PATHWIDGET_H
#define PATHWIDGET_H

#include "ISettingsCategoryWidget.h"
#include <stddef.h>

class   SettingValue;

class   QLineEdit;
class   QPushButton;

class   PathWidget : public ISettingsCategoryWidget
{
    Q_OBJECT

    public:
        PathWidget( SettingValue *s, QWidget *parent = nullptr );
        bool                    save();

    protected:
        void                    changeEvent( QEvent *event );
        void                    retranslate();

    private slots:
        virtual void            changed( const QVariant& );
        void                    selectPathButtonPressed();
    private:
        QLineEdit               *m_lineEdit;
        QPushButton             *m_pushButton;
        QWidget                 *m_widget;
};

#endif // PATHWIDGET_H
