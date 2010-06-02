/*****************************************************************************
 * ShareOnYoutube.h: Configure Youtube Export
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

#ifndef SHAREONYOUTUBE_H
#define SHAREONYOUTUBE_H

#include <QDialog>
#include "ui_ShareOnYoutube.h"

class   ShareOnYoutube : public QDialog
{
    Q_OBJECT

    public:
        ShareOnYoutube();
        QString         username() const;
        QString         password() const;
        QString         title() const;
        QString         category() const;
        QString         description() const;
        QString         tags() const;
        quint32         width() const;
        quint32         height() const;
        bool            videoPrivacy() const;

    private slots:
        virtual void    accept();

    private:
        Ui::ShareOnYoutube    m_ui;
        quint32               m_width;
        quint32               m_height;
};

#endif // SHAREONYOUTUBE_H
