/*****************************************************************************
 * Types.h: Workflow related types.
 *****************************************************************************
 * Copyright (C) 2008-2010 VideoLAN
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
            Frame( quint32 width, quint32 height, size_t forcedSize );
            ~Frame();
            quint32         width() const;
            quint32         height() const;
            quint32         *buffer();
            const quint32   *buffer() const;
            void            setBuffer( quint32 *buff );
            /**
             *  \brief      Resize the buffer.
             *
             *  \warning    This will not resize what's in the frame but drop it instead!
             */
            void            resize( quint32 width, quint32 height );
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
             *  \returns    The frame size in pixels
             *
             *  This is equal to width * height
             */
            quint32         nbPixels() const;
            /**
             *  \warning    Terrible hack !
             *
             *  Remove this ASAP !!
             */
            //FIXME
            qint64      ptsDiff;

        public:
            /**
             * @brief Size  Computes the size, in bytes, a frame with given dimension would use.
             */
            static size_t Size( quint32 width, quint32 height );

        private:
            quint32     m_width;
            quint32     m_height;
            // frei0r uses 32bits only pixels, and expects its buffers as uint32
            quint32     *m_buffer;
            size_t      m_size;
            quint32     m_nbPixels;
    };
    class  AudioSample : public OutputBuffer
    {
        public:
            AudioSample() : OutputBuffer( AudioTrack ){}
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
