/*****************************************************************************
 * ClipMetadataDisplayer.h: Display the basic metadata about a clip.
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

#ifndef CLIPMETADATADISPLAYER_H
#define CLIPMETADATADISPLAYER_H

#include <QWidget>

class   Clip;
class   Media;

#include "ui/ClipMetadataDisplayer.h"

class ClipMetadataDisplayer : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(ClipMetadataDisplayer)

    public:
        explicit ClipMetadataDisplayer( QWidget *parent = 0 );
        virtual ~ClipMetadataDisplayer();

        void                            setWatchedClip( const Clip *clip );
        void                            clear();

    private:
        /**
         *  \brief      Will update the interface depending on the currently displayed
         *              file type.
         */
        void                            updateInterface();
    private:
        Ui::ClipMetadataDisplayer       *m_ui;
        const Clip                      *m_watchedClip;
        QSharedPointer<const Media>     m_watchedMedia;

    private slots:
        void                            metadataUpdated();
        void                            clipDestroyed( Clip* clip );

};

#endif // CLIPMETADATADISPLAYER_H
