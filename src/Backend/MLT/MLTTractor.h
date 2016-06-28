/*****************************************************************************
 * MLTTractor.h:  Wrapper of Mlt::Tractor
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

#ifndef MLTTRACTOR_H
#define MLTTRACTOR_H

#include "Backend/ITractor.h"
#include "MLTInput.h"

namespace Mlt
{
class Tractor;
}

namespace Backend
{
class IProfile;
namespace MLT
{
    class MLTTractor : public ITractor, public MLTInput
    {
    public:
        MLTTractor();
        MLTTractor( IProfile& profile );
        virtual     ~MLTTractor() override;

        virtual void        refresh() override;
        virtual bool        setTrack( IInput& input, int index ) override;
        virtual bool        insertTrack( IInput& input, int index ) override;
        virtual bool        removeTrack( int index ) override;
        virtual IInput*     track( int index ) const override;
        virtual int         count() const override;
        virtual void        addTransition( ITransition& transition, int aTrack, int bTrack ) override;
        virtual void        addFilter( IFilter& filter, int track ) override;
        virtual bool        connect( IInput& input ) override;

    private:
        Mlt::Tractor*      m_tractor;
    };
}
}

#endif // MLTTRACTOR_H
