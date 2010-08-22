/*****************************************************************************
 * Types.h: Workflow related types.
 *****************************************************************************
 * Copyright (C) 2008-2010 VideoLAN
 *
 * Authors: Hugo Beauzée-Luyssen <beauze.h@gmail.com>
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

#ifndef TYPES_H
#define TYPES_H

#include <qglobal.h>

namespace   Workflow
{
    const quint32   Depth = 4;

    /**
     *  \enum   Represents the potential Track types.
     */
    enum    TrackType
    {
        VideoTrack, ///< Represents a video track
        AudioTrack, ///< Represents an audio track
        NbTrackType, ///< Used to know how many types we have
    };

    class   Frame
    {
        public:
            explicit Frame();
            Frame( quint32 width, quint32 height );
            ~Frame();
            quint32         width() const;
            quint32         height() const;
            quint32         *buffer();
            const quint32   *buffer() const;
            void            setBuffer( quint32 *buff );
            Frame           *clone() const;
            /**
             *  \brief      Resize the buffer.
             *
             *  \warning    This will not resize what's in the frame !
             */
            void            resize( quint32 width, quint32 height );
            /**
             *  \returns    The frame size in octets
             *
             *  This is equal to width * height * Depth
             */
            /**
             *  \returns    The frame size in pixels
             *
             *  This is equal to width * height
             */
            quint32         size() const;
            quint32         nbPixels() const;
            /**
             *  \warning    Terrible hack !
             *
             *  Remove this ASAP !!
             */
            //FIXME
            qint64      ptsDiff;
        private:
            quint32     m_width;
            quint32     m_height;
            quint32     *m_buffer;
            quint32     m_size;
            quint32     m_nbPixels;
    };
    struct  AudioSample
    {
        unsigned char*  buff;
        size_t          size;
        quint32         nbSample;
        quint32         nbChannels;
        qint64          ptsDiff;
        qint64          pts;
    };
}

#endif // TYPES_H
