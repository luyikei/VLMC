/*****************************************************************************
 * ISettingsCategorieWidget.h: Common interface for settings widgets.
 *****************************************************************************
 * Copyright (C) 2008-2010 VideoLAN
 *
 * Authors: Hugo Beauzee-Luyssen <hugo@vlmc.org>
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


#ifndef ISETTINGSCATEGORYWIDGET_H
#define ISETTINGSCATEGORYWIDGET_H

class   QVariant;
class   QWidget;

#include <QObject>

class   ISettingsCategoryWidget : public QObject
{
    Q_OBJECT

    public:
        virtual QWidget*        widget() = 0;
        virtual void            save() = 0;

    protected slots:
        virtual void            changed( const QVariant& ) = 0;
};

#endif // ISETTINGSCATEGORYWIDGET_H
