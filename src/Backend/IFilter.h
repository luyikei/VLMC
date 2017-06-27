/*****************************************************************************
 * IFilter.h: Defines an interface of a filter
 *****************************************************************************
 * Copyright (C) 2008-2016 VideoLAN
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

#ifndef IFILTER_H
#define IFILTER_H

#include <cstdint>
#include <string>
#include <vector>
#include <memory>

namespace Backend
{
    class IInput;
    class IInfo;

    class IFilter
    {
    public:
        virtual ~IFilter() = default;


        virtual std::string     identifier() const = 0;
        virtual bool            connect( IInput& input, int index = 0 ) = 0;
        virtual void            setBoundaries( int64_t begin, int64_t end ) = 0;
        virtual int64_t         begin() const = 0;
        virtual int64_t         end() const = 0;
        virtual int64_t         length() const = 0;
        virtual void            detach() = 0;

        virtual std::shared_ptr<IInput> input() const = 0;

        virtual const IInfo&    filterInfo() const = 0;
    };
}

#endif // IFILTER_H
