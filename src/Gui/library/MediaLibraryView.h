/*****************************************************************************
 * MediaLibrary.h: VLMC media library's ui
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

#ifndef MEDIALIBRARYVIEW_H
#define MEDIALIBRARYVIEW_H

#include <QWidget>

class MediaLibraryView : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY( MediaLibraryView )

    public:
        explicit MediaLibraryView( QWidget* parent = 0);
        virtual ~MediaLibraryView();

        QWidget*    container();

    public slots:
        void    startDrag( qint64 mediaId );

    private:
        QWidget*    m_container;
};

#endif // MEDIALIBRARYVIEW_H
