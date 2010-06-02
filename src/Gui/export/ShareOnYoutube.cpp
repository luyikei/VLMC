/*****************************************************************************
 * ShareOnYoutube.cpp: Configure Youtube Export
 *****************************************************************************
 * Copyright (C) 2008-2010 VideoLAN
 *
 * Authors: Rohit Yadav <rohityadav89@gmail.com>
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

#include "ShareOnYoutube.h"
#include "SettingsManager.h"

#include "ui_ShareOnYoutube.h"

ShareOnYoutube::ShareOnYoutube()
{
    m_ui.setupUi( this );
    m_ui.title->setText( VLMC_PROJECT_GET_STRING( "general/ProjectName" ) );
}

void
ShareOnYoutube::accept()
{
    switch( m_ui.videoSize->currentIndex() )
    {
        case 0:  m_width = 480;  m_height = 272; break;
        case 1:  m_width = 640;  m_height = 360; break;
        case 2:  m_width = 960;  m_height = 540; break;
        case 3:  m_width = 1280; m_height = 720; break;
        default: m_width = 640;  m_height = 360;
    }

    //Error checks here
    QDialog::accept();
}

QString
ShareOnYoutube::username() const
{
    return m_ui.username->text();
}

QString
ShareOnYoutube::password() const
{
    return m_ui.password->text();
}

QString
ShareOnYoutube::category() const
{
    return m_ui.category->currentText();
}

QString
ShareOnYoutube::title() const
{
    return m_ui.title->text();
}

QString
ShareOnYoutube::description() const
{
    return m_ui.description->toPlainText();
}

QString
ShareOnYoutube::tags() const
{
    return m_ui.tags->text();
}

quint32
ShareOnYoutube::width() const
{  
    return m_width;
}

quint32
ShareOnYoutube::height() const
{
    return m_height;
}

bool
ShareOnYoutube::videoPrivacy() const
{
    return m_ui.videoPrivacy->isChecked();
}

