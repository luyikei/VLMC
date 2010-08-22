/*****************************************************************************
 * Types.cpp: Workflow related types.
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

#include "Workflow/Types.h"

using namespace Workflow;

#include <cstring> //memcpy

Frame::Frame() :
        OutputBuffer( VideoTrack ),
        ptsDiff( 0 ),
        m_width( 0 ),
        m_height( 0 ),
        m_buffer( 0 ),
        m_size( 0 ),
        m_nbPixels( 0 )
{
}

Frame::Frame( quint32 width, quint32 height ) :
        OutputBuffer( VideoTrack ),
        ptsDiff( 0 ),
        m_width( width ),
        m_height( height )
{
    m_nbPixels = width * height;
    m_size = m_nbPixels * Depth;
    m_buffer = new quint32[m_nbPixels];
}

Frame::~Frame()
{
    delete[] m_buffer;
}

quint32*
Frame::buffer()
{
    return m_buffer;
}

const quint32*
Frame::buffer() const
{
    return m_buffer;
}

quint32
Frame::width() const
{
    return m_width;
}

quint32
Frame::height() const
{
    return m_height;
}

quint32
Frame::size() const
{
    return m_size;
}

quint32
Frame::nbPixels() const
{
    return m_nbPixels;
}

Frame*
Frame::clone() const
{
    Frame   *f = new Frame( m_width, m_height );
    memcpy( f->buffer(), m_buffer, m_size );
    return f;
}

void
Frame::setBuffer( quint32 *buff )
{
    delete[] m_buffer;
    m_buffer = buff;
}

void
Frame::resize( quint32 width, quint32 height )
{
    if ( width != m_width || height != m_height )
    {
        delete[] m_buffer;
        m_width = width;
        m_height = height;
        m_nbPixels = width * height;
        m_size = m_nbPixels * Depth;
        m_buffer = new quint32[m_nbPixels];
    }
}
