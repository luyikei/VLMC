/*****************************************************************************
 * MLTProducer.h:  Wrapper of Mlt::Producer
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

#ifndef MLTPRODUCER_H
#define MLTPRODUCER_H

#include "Backend/IProducer.h"
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

class MLTProducer : virtual public IProducer, public MLTService
{
    public:

        MLTProducer( Mlt::Producer* producer, IProducerEventCb* callback = nullptr );
        MLTProducer( const char* path, IProducerEventCb* callback = nullptr );
        MLTProducer( IProfile& profile, const char* path, IProducerEventCb* callback = nullptr );
        ~MLTProducer();

        static void             onPropertyChanged( void* owner, MLTProducer* self, const char* id );

        virtual void            setCallback( IProducerEventCb* callback );

        virtual const char*     path() const override;
        virtual int64_t         begin() const override;
        virtual int64_t         end() const override;
        virtual void            setBegin( int64_t begin ) override;
        virtual void            setEnd( int64_t end ) override;
        virtual void            setBoundaries( int64_t begin, int64_t end ) override;

        virtual IProducer*      cut( int64_t begin = 0, int64_t end = EndOfMedia ) override;
        virtual bool            isCut() const override;

        virtual bool            sameClip( IProducer& that ) const override;
        virtual bool            runsInto( IProducer& that ) const override;

        // The playable (based on begin and end points) duration
        virtual int64_t         playableLength() const override;

        // The duration of the producer regardless of begin and end points.
        virtual int64_t         length() const override;
        virtual const char*     lengthTime() const override;

        // The position in frame relative to its beginning
        virtual int64_t         position() const override;
        virtual void            setPosition( int64_t position ) override;

        // The absolete position in frame
        virtual int64_t         frame() const override;

        virtual double          fps() const override;
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
    protected:
        MLTProducer();

        void                    calcTracks();

        Mlt::Producer*          m_producer;
        IProducerEventCb*       m_callback;
        bool                    m_paused;

        int                     m_nbVideoTracks;
        int                     m_nbAudioTracks;

    friend class MLTConsumer;
    friend class MLTTrack;
    friend class MLTTractor;
};

}
}

#endif // MLTPRODUCER_H
