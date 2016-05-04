/*****************************************************************************
 * Types.h: Workflow related types.
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

#ifndef TYPES_H
#define TYPES_H

#include <qglobal.h>

namespace   Workflow
{
    // This is constrained by frei0r
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

    struct  OutputBuffer
    {
        TrackType   type;
        protected:
            OutputBuffer( TrackType _type ) : type( _type ) {}
    };

    class   Frame : public OutputBuffer
    {
        public:
            explicit Frame();
            Frame( quint32 width, quint32 height );
            Frame( size_t forcedSize );
            ~Frame();
            quint32         *buffer();
            const quint32   *buffer() const;
            void            setBuffer( quint32 *buff );
            /**
              * \brief      Resize the buffer
              *
              * This will allocate a new buffer and drop the old one.
              * \param      size    The size, in bytes.
              */
            void            resize( size_t size );
            /**
             *  \returns    The frame size in octets
             *
             *  This is equal to width * height * Depth
             */
            size_t          size() const;
            /**
             *  \brief      Get pts.
             *
             */
            qint64          pts() const;
            /**
             *  \brief      Set pts.
             *
             */
            void           setPts( qint64 pts );
            /**
             *  \warning    Terrible hack !
             *
             *  Remove this ASAP !!
             */
            //FIXME
            qint64      ptsDiff;

        private:
            // frei0r uses 32bits only pixels, and expects its buffers as uint32
            quint32     *m_buffer;
            size_t      m_size;
            quint32     m_nbPixels;
            qint64      m_pts;
    };
    class  AudioSample : public OutputBuffer
    {
        public:
            AudioSample()
                : OutputBuffer( AudioTrack )
                , buff( nullptr )
                , size( 0 )
            {
            }
            unsigned char*  buff;
            size_t          size;
            quint32         nbSample;
            quint32         nbChannels;
            qint64          ptsDiff;
            qint64          pts;
    };
}

namespace   Vlmc
{
    /**
     *  \enum   Used to know which part required a change of rendered frame.
     *          The main use of this enum is to avoid infinite information propagation
     *          such as the timeline informing itself that the frame as changed, which
     *          would cause a signal to be emmited to inform every other part that the
     *          rendered frame has changed, and so on.
     */
    enum    FrameChangedReason
    {
        Renderer, ///< Used by the WorkflowRenderer
        /**
         *  \brief      Used by the timeline cursor.
         *  \warning    The timeline cursor is not the timeline ruler
         */
        TimelineCursor,
        PreviewCursor, ///< Used by the preview widget, when using the time cursor.
        RulerCursor, ///< Used by the timeline's ruler.
    };
}

#endif // TYPES_H
