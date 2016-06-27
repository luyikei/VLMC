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

#include "ui/MediaLibraryView.h"

class   Clip;
class   MediaListView;
class   MediaContainer;
class   ViewController;

class MediaLibraryView : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY( MediaLibraryView )

    public:
        typedef bool    (*Filter)( const Clip*, const QString& filter );
        explicit MediaLibraryView( QWidget *parent = 0);
        virtual ~MediaLibraryView();

    protected:
        void        dragEnterEvent( QDragEnterEvent *event );
        void        dragMoveEvent( QDragMoveEvent *event );
        void        dragLeaveEvent( QDragLeaveEvent *event );
        void        dropEvent( QDropEvent *event );
        void        changeEvent( QEvent *e );

    private:
        /**
         *  \return     The appropriate filter function
         *
         *  This will evaluate the currently selected filter, and return the appropriate
         *  function.
         */
        Filter              currentFilter();

    //Filters list :
        static bool         filterByName( const Clip *clip, const QString &filter );
        static bool         filterByTags( const Clip *clip, const QString &filter );

    private:
        Ui::MediaLibraryView *m_ui;
        MediaListView        *m_mediaListView;

    private slots:
        void                filterUpdated( const QString &filter );
        /**
         *  \brief          Used to update the filters when the view is changed.
         *
         *  A view is changed when the user goes through the clips hierarchy.
         */
        void                viewChanged( ViewController* view );
        void                filterTypeChanged();

    signals:
        void                clipSelected( Clip* );
};

#endif // MEDIALIBRARYVIEW_H
