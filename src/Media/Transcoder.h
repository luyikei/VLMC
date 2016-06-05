/*****************************************************************************
 * Transcoder.h: Handle file transcoding.
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

#ifndef TRANSCODER_H
#define TRANSCODER_H

#include <QObject>

#include "Media.h"

namespace Backend
{
namespace VLC
{
class VLCSourceRenderer;
class RendererEventWatcher;
}
}

class RendererEventWatcher;

class Transcoder : public QObject
{
    Q_OBJECT
    public:
        explicit    Transcoder( Media *media );
        ~Transcoder();
        void        transcodeToPs();

    private:
        Media*                      m_media;
        QString                     m_destinationFile;
        Backend::VLC::VLCSourceRenderer*   m_renderer;
        Backend::VLC::RendererEventWatcher*       m_eventWatcher;

    private slots:
        void        transcodeFinished();

    signals:
        void        progress( int percent );
        void        done();
        //used for notification:
        void        notify( QString );
};

#endif // TRANSCODER_H
