/*****************************************************************************
 * Transcoder.h: Handle file transcoding.
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

#ifndef TRANSCODER_H
#define TRANSCODER_H

#include <QObject>

#include "Media.h"

class Transcoder : public QObject
{
    Q_OBJECT
    public:
        explicit    Transcoder( Media *media );
        void        transcodeToPs();

    private:
        Media       *m_media;
        QString     m_destinationFile;

    private slots:
        void        transcodeFinished();

    signals:
        void        progress( float pos );
        void        done();
};

#endif // TRANSCODER_H
