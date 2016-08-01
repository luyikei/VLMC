/*****************************************************************************
 * MLTInput.h:  Wrapper of Mlt::Producer
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

#ifndef MLTINPUT_H
#define MLTINPUT_H

#include "Backend/IInput.h"
#include "Backend/IProfile.h"
#include "MLTService.h"

namespace Mlt
{
class Producer;
}

namespace Backend
{
namespace MLT
{

class MLTInput : virtual public IInput, public MLTService
{
    public:

        MLTInput( Mlt::Producer* input, IInputEventCb* callback = nullptr );
        MLTInput( const char* path, IInputEventCb* callback = nullptr );
        MLTInput( IProfile& profile, const char* path, IInputEventCb* callback = nullptr );
        ~MLTInput();

        virtual Mlt::Producer*  producer();
        virtual Mlt::Producer*  producer() const;

        virtual Mlt::Service*   service() override;
        virtual Mlt::Service*   service() const override;

        static void             onPropertyChanged( void* owner, MLTInput* self, const char* id );

        virtual void            setCallback( IInputEventCb* callback ) override;

        virtual const char*     path() const override;
        virtual int64_t         begin() const override;
        virtual int64_t         end() const override;
        virtual void            setBegin( int64_t begin ) override;
        virtual void            setEnd( int64_t end ) override;
        virtual void            setBoundaries( int64_t begin, int64_t end ) override;

        virtual std::unique_ptr<IInput>      cut( int64_t begin = 0, int64_t end = EndOfMedia ) override;
        virtual bool            isCut() const override;

        virtual bool            sameClip( IInput& that ) const override;
        virtual bool            runsInto( IInput& that ) const override;

        // The playable (based on begin and end points) duration
        virtual int64_t         playableLength() const override;

        // The duration of the input regardless of begin and end points.
        virtual int64_t         length() const override;
        virtual const char*     lengthTime() const override;

        // The position in frame relative to its beginning
        virtual int64_t         position() const override;
        virtual void            setPosition( int64_t position ) override;

        // The absolete position in frame
        virtual int64_t         frame() const override;

        // Generates an 8-bit grayscale image at the current position
        virtual uint8_t*        waveform( uint32_t width, uint32_t height ) const override;

        // Generates an 32-bit RGBA image at the current position
        virtual uint8_t*        image( uint32_t width, uint32_t height ) const override;

        virtual double          fps() const override;
        virtual double          aspectRatio() const override;
        virtual int             width() const override;
        virtual int             height() const override;
        virtual bool            hasVideo() const override;
        virtual int             nbVideoTracks() const override;
        virtual bool            hasAudio() const override;
        virtual int             nbAudioTracks() const override;

        virtual void            playPause() override;
        virtual bool            isPaused() const override;
        virtual void            setPause( bool isPaused ) override;
        virtual void            nextFrame() override;
        virtual void            previousFrame() override;

        virtual bool            isBlank() const override;

        virtual bool            attach( IFilter& filter ) override;
        virtual bool            detach( IFilter& filter ) override;
        virtual bool            detach( int index ) override;
        virtual int             filterCount() const override;
        virtual bool            moveFilter( int from, int to ) override;
        virtual std::shared_ptr<IFilter>  filter( int index ) const override;
    protected:
        MLTInput();

        void                    calcTracks();

    private:
        Mlt::Producer*          m_producer;
        IInputEventCb*          m_callback;
        bool                    m_paused;

        int                     m_nbVideoTracks;
        int                     m_nbAudioTracks;
};

}
}

#endif // MLTINPUT_H
