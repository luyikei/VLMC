/*****************************************************************************
 * IBackend.h: Provides an entry point to a backend functionnalities
 *****************************************************************************
 * Copyright (C) 2008-2016 VideoLAN
 *
 * Authors: Yikei Lu <luyikei.qmltu@gmail.com>
 *          Hugo Beauzée-Luyssen <hugo@beauzee.fr>
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

#ifndef IBACKEND_H
#define IBACKEND_H

#include <functional>

#include <string>
#include <map>

namespace Backend
{

class IOutput;
class IProfile;
class IInfo;

class IBackend
{
    public:
        enum LogLevel
        {
            Debug,
            Warning,
            Error,
            None
        };
        using LogHandler = std::function<void( LogLevel logLevel, const char* msg )>;

        virtual ~IBackend() = default;
        virtual IProfile&                   profile() = 0;
        virtual const std::map<std::string, IInfo*>&          availableFilters() const = 0;
        virtual const std::map<std::string, IInfo*>&          availableTransitions() const = 0;
        virtual IInfo*                                        filterInfo( const std::string& id ) const = 0;
        virtual IInfo*                                        transitionInfo( const std::string& id ) const = 0;

        virtual void                        setLogHandler( LogHandler logHandler ) = 0;
};

extern IBackend* instance();
}

#endif // IBACKEND_H
