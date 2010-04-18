/*****************************************************************************
 * ClipHelper.cpp: Contains information about a Clip in the workflow
 *****************************************************************************
 * Copyright (C) 2008-2010 VideoLAN
 *
 * Authors: Hugo Beauzée-Luyssen <hugo@vlmc.org>
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

#include "ClipHelper.h"

#include "Clip.h"

ClipHelper::ClipHelper( Clip* clip, qint64 begin /*= -1*/, qint64 end /*= -1*/ ) :
        m_clip( clip ),
        m_begin( begin ),
        m_end( end )
{
    if ( begin == -1 )
        m_begin = clip->begin();
    if ( end == -1 )
        m_end = clip->end();
    m_uuid = QUuid::createUuid();
}

void
ClipHelper::setBegin( qint64 begin )
{
    if ( begin <= m_clip->m_begin )
        return ;
    m_begin = begin;
    emit lengthUpdated();
}

void
ClipHelper::setEnd( qint64 end )
{
    if ( end >= m_clip->m_end )
        return ;
    m_end = end;
    emit lengthUpdated();
}

void
ClipHelper::setBoundaries( qint64 begin, qint64 end )
{
    if ( begin >= m_clip->m_begin )
        m_begin = begin;
    if ( end <= m_clip->m_end )
        m_end = end;
    emit lengthUpdated();
}

qint64
ClipHelper::length() const
{
    return m_end - m_begin;
}
