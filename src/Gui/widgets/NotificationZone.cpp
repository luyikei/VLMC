/*****************************************************************************
 * NotificationZone.cpp: Handle toolbar notification zone.
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "NotificationZone.h"

#include <QTimer>

NotificationZone::NotificationZone(QWidget *parent) :
    QWidget(parent)
{
    m_ui = new Ui::NotificationZone();
    m_ui->setupUi( this );
    m_ui->container->hide();
    m_timer = new QTimer( this );
    m_timer->setSingleShot( true );
    m_timer->setInterval( 5000 );
    connect( m_timer, SIGNAL( timeout() ), this, SLOT( hideNotification() ) );
}

NotificationZone::~NotificationZone()
{
    delete m_ui;
}

void
NotificationZone::notify( const QString &message )
{
    m_ui->progressBar->hide();
    m_ui->container->show();
    m_ui->message->setText( message );
    m_timer->start();
}

void
NotificationZone::progressUpdated( int percent )
{
    m_ui->progressBar->show();
    m_ui->progressBar->setValue( percent );
    if ( percent >= 100 )
        m_timer->start();
    else
        m_timer->stop();
}

void
NotificationZone::hideNotification()
{
    //This could be direct connected, but i'd like to make it fade out one day :)
    m_ui->container->hide();
}
