/*****************************************************************************
 * MediaLibrary.h: VLMC media library's ui
 *****************************************************************************
 * Copyright (C) 2008-2010 VideoLAN
 *
 * Authors: Hugo Beauzee-Luyssen <beauze.h@gmail.com>
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

#ifndef MEDIALIBRARY_H
#define MEDIALIBRARY_H

#include <QWidget>

#include "ui_MediaLibrary.h"
class   Clip;

class MediaLibrary : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY( MediaLibrary );

    public:
        explicit MediaLibrary( QWidget *parent = 0);

    private:
        Ui::MediaLibrary    *m_ui;

    private slots:
        void                filterUpdated( const QString &filter );
        void                clearFilter();

    signals:
        void                importRequired();
        void                clipSelected( Clip* );
};

#endif // MEDIALIBRARY_H
