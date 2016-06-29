/*****************************************************************************
 * MLTTrack.h:  Wrapper of Mlt::Track
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

#ifndef MLTTRACK_H
#define MLTTRACK_H


#include "MLTInput.h"
#include "Backend/ITrack.h"

namespace Mlt
{
class Playlist;
}

namespace Backend
{
class IProfile;
namespace MLT
{

class MLTTrack : public ITrack, public MLTInput
{
    public:
        MLTTrack();
        MLTTrack( IProfile& profile );
        virtual ~MLTTrack();

        virtual Mlt::Playlist*  playlist();
        virtual Mlt::Playlist*  playlist() const;

        virtual Mlt::Producer*  producer() override;
        virtual Mlt::Producer*  producer() const override;

        virtual bool        insertAt( IInput& input, int64_t startFrame ) override;
        virtual bool        append( IInput& input ) override;
        virtual bool        remove( int index ) override;
        virtual bool        move( int src, int dist ) override;
        virtual IInput*  clip( int index ) const override;
        virtual IInput*  clipAt( int64_t position ) const override;
        virtual bool        resizeClip( int clip, int64_t begin, int64_t end ) override;
        virtual int         clipIndexAt( int64_t position ) override;
        virtual int         count() const override;
        virtual void        clear();

        virtual void        setMute( bool muted ) override;
        virtual void        setVideoEnabled( bool enabled ) override;

    private:
        Mlt::Playlist*                  m_playlist;
};

}
}

#endif // MLTTRACK_H
