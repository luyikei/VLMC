/*****************************************************************************
 * IBackend.h: Provides an entry point to a backend functionnalities
 *****************************************************************************
 * Copyright (C) 2008-2014 VideoLAN
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

#ifndef IBACKEND_H
#define IBACKEND_H

#include <functional>

namespace Backend
{

class ISourceRenderer;
class ISource;
class IMemorySource;

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
        virtual ISource*        createSource( const char* path ) = 0;
        virtual IMemorySource*  createMemorySource() = 0;
        virtual void            setLogHandler( LogHandler logHandler ) = 0;
};

extern IBackend* getBackend();

}

#endif // IBACKEND_H
