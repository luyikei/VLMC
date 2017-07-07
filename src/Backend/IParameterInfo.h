/*****************************************************************************
 * IParameterInfo.h:  Abstract parameter information class
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

#ifndef IPARAMETERINFO_H
#define IPARAMETERINFO_H

#include <string>

namespace Backend
{
    class IParameterInfo
    {
    public:
        virtual ~IParameterInfo() = default;

        virtual const std::string&  identifier() const = 0;
        virtual const std::string&  name() const = 0;
        virtual const std::string&  type() const = 0;
        virtual const std::string&  description() const = 0;

        virtual const std::string&   defaultValue() const = 0;
        virtual const std::string&   minValue() const = 0;
        virtual const std::string&   maxValue() const = 0;
    };
}

#endif // IPARAMETERINFO_H
