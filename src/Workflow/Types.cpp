/*****************************************************************************
 * Types.cpp: Workflow related types.
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

#include "Workflow/Types.h"

using namespace Workflow;

#include <cstring> //memcpy

Frame::Frame() :
        OutputBuffer( VideoTrack ),
        ptsDiff( 0 ),
        m_buffer( 0 ),
        m_size( 0 ),
        m_pts( 0 )
{
}

Frame::Frame( quint32 width, quint32 height ) :
        OutputBuffer( VideoTrack ),
        ptsDiff( 0 ),
        m_pts( 0 )
{
    m_size = width * height * Depth;
    m_buffer = new quint32[width * height];
}

Frame::Frame( size_t forcedSize ) :
    OutputBuffer( VideoTrack ),
    ptsDiff( 0 ),
    m_pts( 0 )
{
    m_size = forcedSize;
    m_buffer = new quint32[ ( forcedSize % 4 ) ? forcedSize / 4 + 1 : forcedSize / 4 ];
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

const quint32 *Frame::buffer() const
{
    return m_buffer;
}

size_t Frame::size() const
{
    return m_size;
}

qint64
Frame::pts() const
{
    return m_pts;
}

void
Frame::setPts( qint64 pts )
{
    m_pts = pts;
}

void
Frame::setBuffer( quint32 *buff )
{
    delete[] m_buffer;
    m_buffer = buff;
}

void
Frame::resize( size_t size )
{
    if ( size == m_size )
        return ;
    delete[] m_buffer;
    m_size = size;
    m_buffer = new quint32[ ( size % 4 ) ? size / 4 + 1 : size / 4  ];
}
