/*****************************************************************************
 * IInfo.h:  Abstract information class
 *****************************************************************************
 * Copyright (C) 2008-2017 VideoLAN
 *
 * Authors: Yikei Lu <luyikei.qmltu@gmail.com>
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

#ifndef IINFO_H
#define IINFO_H

#include <vector>

#include "IParameterInfo.h"

namespace Backend
{
class IInfo
{
public:
    virtual ~IInfo() = default;

    virtual const std::string&  identifier() const = 0;
    virtual const std::string&  name() const = 0;
    virtual const std::string&  description() const = 0;
    virtual const std::string&  author() const = 0;
    virtual const std::vector<IParameterInfo*>& paramInfos() const = 0;
};
}

#endif // IINFO_H
