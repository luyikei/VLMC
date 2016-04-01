/*****************************************************************************
 * ISettingsCategorieWidget.h: Common interface for settings widgets.
 *****************************************************************************
 * Copyright (C) 2008-2016 VideoLAN
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

#include "Settings/SettingValue.h"

class   QVariant;

#include <QWidget>
#include <QHBoxLayout>

class   ISettingsCategoryWidget : public QWidget
{
    Q_OBJECT

    public:
        virtual ~ISettingsCategoryWidget(){}
        virtual SettingValue    *setting() { return m_setting; }
        virtual bool            save() = 0;

    protected:
        ISettingsCategoryWidget( QWidget *parent, SettingValue* s ) :
                QWidget( parent ),
                m_setting( s )
        {
            connect( s, SIGNAL( changed( const QVariant& ) ),
                     this, SLOT( changed( const QVariant& ) ) );
            QHBoxLayout *layout = new QHBoxLayout;
            setLayout( layout );
        }

    protected:
        SettingValue            *m_setting;

    public slots:
        virtual void            changed( const QVariant& ) = 0;
};

#endif // ISETTINGSCATEGORYWIDGET_H
