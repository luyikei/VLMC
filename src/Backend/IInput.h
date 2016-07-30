/*****************************************************************************
 * IInput.h: Defines an interface to produce frames
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

#ifndef IINPUT_H
#define IINPUT_H

#include <cstdint>
#include <memory>

namespace Backend
{
    class IFilter;
    class IInputEventCb
    {
    public:
        virtual ~IInputEventCb() = default;
        virtual void    onPaused() = 0;
        virtual void    onPlaying() = 0;
        virtual void    onEndReached() = 0;
        virtual void    onPositionChanged( int64_t ) = 0;
        virtual void    onLengthChanged( int64_t ) = 0;
        virtual void    onErrorEncountered() = 0;
    };

    class IInput
    {
    public:
        enum EndType
        {
            EndOfMedia = -1,
            EndOfParent = -2,
            Unlimited = -3
        };

        virtual ~IInput() = default;
        virtual void            setCallback( IInputEventCb* callback ) = 0;

        virtual const char*     path() const = 0;
        virtual int64_t         begin() const = 0;
        virtual int64_t         end() const = 0;
        virtual void            setBegin( int64_t begin ) = 0;
        virtual void            setEnd( int64_t end ) = 0;
        virtual void            setBoundaries( int64_t begin, int64_t end ) = 0;

        // Absolute position in frames
        virtual std::unique_ptr<IInput>      cut( int64_t begin  = 0, int64_t end  = EndOfMedia ) = 0;
        virtual bool            isCut( ) const = 0 ;

        virtual bool            sameClip( IInput& that ) const = 0;
        virtual bool            runsInto( IInput& that ) const = 0;

        // The playable (based on begin and end points) duration
        virtual int64_t         playableLength() const = 0;

        // The duration of the input regardless of begin and end points.
        virtual int64_t         length() const = 0;
        virtual const char*     lengthTime() const = 0;

        // The position in frame relative to its beginning
        virtual int64_t         position() const = 0;
        virtual void            setPosition( int64_t position ) = 0;

        // The absolete position in frame
        virtual int64_t         frame() const = 0;

        // Generates an 8-bit grayscale image at the current position
        virtual uint8_t*        waveform( uint32_t width, uint32_t height ) const = 0;

        // Generates an 32-bit RGBA image at the current position
        virtual uint8_t*        image( uint32_t width, uint32_t height ) const = 0;

        virtual double          fps() const = 0;
        virtual double          aspectRatio() const = 0;
        virtual int             width() const = 0;
        virtual int             height() const = 0;
        virtual bool            hasVideo() const = 0;
        virtual int             nbVideoTracks() const = 0;
        virtual bool            hasAudio() const = 0;
        virtual int             nbAudioTracks() const = 0;

        virtual void            playPause() = 0;
        virtual void            setPause( bool isPaused ) = 0;
        virtual bool            isPaused() const = 0;
        virtual void            nextFrame() = 0;
        virtual void            previousFrame() = 0;

        virtual bool            isBlank() const = 0;

        virtual bool            attach( IFilter& filter ) = 0;
        virtual bool            detach( IFilter& filter ) = 0;
        virtual bool            detach( int index ) = 0;
        virtual int             filterCount() const = 0;
        virtual bool            moveFilter( int from, int to ) = 0;
        virtual std::shared_ptr<IFilter>  filter( int index ) const = 0;
    };
}

#endif // IINPUT_H
