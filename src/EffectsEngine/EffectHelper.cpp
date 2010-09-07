/*****************************************************************************
 * EffectHelper: Contains informations about effects
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

#include "EffectHelper.h"

EffectHelper::EffectHelper( EffectInstance *effectInstance, qint64 begin, qint64 end,
                            const QString &uuid ) :
    m_effectInstance( effectInstance ),
    m_begin( begin ),
    m_end( end )
{
    if ( uuid.isNull() == true )
        m_uuid = QUuid::createUuid();
    else
        m_uuid = uuid;
}

qint64
EffectHelper::begin() const
{
    return m_begin;
}

qint64
EffectHelper::end() const
{
    return m_end;
}

void
EffectHelper::setBegin( qint64 begin )
{
    m_begin = begin;
    emit lengthUpdated();
}

void
EffectHelper::setEnd(qint64 end)
{
    m_end = end;
    emit lengthUpdated();
}

void
EffectHelper::setBoundaries( qint64 begin, qint64 end )
{
    m_begin = begin;
    m_end = end;
    emit lengthUpdated();
}

qint64
EffectHelper::length() const
{
    return m_end - m_begin;
}

const QUuid&
EffectHelper::uuid() const
{
    return m_uuid;
}

EffectInstance*
EffectHelper::effectInstance()
{
    return m_effectInstance;
}

const EffectInstance*
EffectHelper::effectInstance() const
{
    return m_effectInstance;
}
