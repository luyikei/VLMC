/*****************************************************************************
 * MLTTransition.h:  Wrapper of Mlt::Transition
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

#ifndef MLTTRANSITION_H
#define MLTTRANSITION_H

#include "Backend/ITransition.h"
#include "MLTService.h"

namespace Mlt
{
class Transition;
}

namespace Backend
{
class IProfile;
namespace MLT
{
    class MLTTransition : public ITransition, public MLTService
    {
    public:
        MLTTransition( IProfile& profile, const char* id );
        MLTTransition( const char* id );
        virtual ~MLTTransition() override;

        virtual Mlt::Transition*  transition();
        virtual Mlt::Transition*  transition() const;

        virtual Mlt::Service*   service() override;
        virtual Mlt::Service*   service() const override;

        virtual void    setBoundaries( int64_t begin, int64_t end ) override;
        virtual int64_t begin() const override;
        virtual int64_t end() const override;
        virtual int64_t length() const override;

    private:
        Mlt::Transition*      m_transition;
    };
}
}

#endif // MLTTRANSITION_H
