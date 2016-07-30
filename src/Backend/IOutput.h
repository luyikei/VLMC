/*****************************************************************************
 * IOutput: Defines an interface to control a player
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

#ifndef IOUTPUT_H
#define IOUTPUT_H

#include <string>
#include <cstdint>
#include <cstddef>

namespace Backend
{
    class IInput;

    class IOutputEventCb
    {
    public:
        virtual ~IOutputEventCb() = default;
        virtual void    onPlaying() = 0;
        virtual void    onStopped() = 0;
        virtual void    onVolumeChanged() = 0;
        virtual void    onErrorEncountered() = 0;
    };

    class IOutput
    {
    public:
        virtual ~IOutput() = default;

        virtual void    setName( const char* name ) = 0;
        virtual void    setCallback( IOutputEventCb* callback ) = 0;
        /**
         * @brief start Initializes and launches playback.
         */
        virtual void    start() = 0;
        virtual void    stop() = 0;
        virtual bool    isStopped() const = 0;

        virtual int     volume() const = 0;
        virtual void    setVolume( int volume ) = 0;

        virtual bool    connect( IInput& input ) = 0;
        virtual bool    isConnected() const = 0;
    };
}

#endif // IOUTPUT_H
