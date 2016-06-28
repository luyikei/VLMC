/*****************************************************************************
 * IMultiTrack.h: Defines an interface of a multitrack input
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

#ifndef IMULTITRACK_H
#define IMULTITRACK_H

#include "IInput.h"

namespace Backend
{
    class IInput;
    class ITransition;
    class IFilter;
    class IMultiTrack : virtual public IInput
    {
    public:
        virtual     ~IMultiTrack() = default;

        virtual void        refresh() = 0;
        virtual bool        setTrack( IInput& input, int index ) = 0;
        virtual bool        insertTrack( IInput& input, int index ) = 0;
        virtual bool        removeTrack( int index ) = 0;
        virtual IInput*     track( int index ) const = 0;
        virtual int         count() const = 0;
        virtual void        addTransition( ITransition& transition, int aTrack = 0, int bTrack = 1 ) = 0;
        virtual void        addFilter( IFilter& filter, int track = 0 ) = 0;
        virtual bool        connect( IInput& input ) = 0;
    };
}

#endif // IMULTITRACK_H
