/*****************************************************************************
 * NotificationZone.h: Handle toolbar notification zone.
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

#ifndef NOTIFICATIONZONE_H
#define NOTIFICATIONZONE_H

#include "Tools/Singleton.hpp"

class   QTimer;

#include <QWidget>

#include "ui/NotificationZone.h"

class NotificationZone : public QWidget, public Singleton<NotificationZone>
{
    Q_OBJECT

    public:

    private:
        explicit NotificationZone(QWidget *parent = 0);
        virtual ~NotificationZone();

    private:
        Ui::NotificationZone    *m_ui;
        QTimer                  *m_timer;

    public slots:
        void        notify( const QString& message );
        /**
         *  \brief  Update the progress bar.
         *
         *  \param  ratio   The progress ratio, from 0.0 to 1.0
         */
        void        progressUpdated( float ratio );
        /**
         *  \brief  Update the progress bar.
         *
         *  \param  percent The progress percent, from 0 to 100
         */
        void        progressUpdated( int percent );

    private slots:
        void        hideNotification();

    friend Singleton_t::AllowInstantiation;
};

#endif // NOTIFICATIONZONE_H
