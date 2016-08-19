/*****************************************************************************
 * Helper.cpp: Describes a common interface for all workflow helpers.
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "Helper.h"

using namespace Workflow;

Helper::Helper( const QUuid& uuid/* = QUuid()*/ )
{
    if ( uuid.isNull() == true )
        m_uuid = QUuid::createUuid();
    else
        m_uuid = uuid;
}

Helper::~Helper()
{
}

const QUuid&
Helper::uuid() const
{
    return m_uuid;
}
