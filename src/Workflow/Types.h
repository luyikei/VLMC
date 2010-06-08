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
    const quint32   Depth = 3;

    class   Frame
    {
        public:
            Frame( quint32 width, quint32 height );
            ~Frame();
            quint32         width() const;
            quint32         height() const;
            quint8          *buffer();
            const quint8    *buffer() const;
            /**
             *  \returns    The frame size in pixels
             *
             *  This is equal to width * height * Depth
             */
            quint32         size() const;
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
            quint8      *m_buffer;
            quint32     m_size;
    };
}

#endif // TYPES_H
