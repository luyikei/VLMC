/*****************************************************************************
 * MLTConsumer.h:  Wrapper of Mlt::Consumer
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

#ifndef MLTCONSUMER_H
#define MLTCONSUMER_H

#include "MLTService.h"
#include "Backend/IConsumer.h"
#include "Backend/IBackend.h"
#include "Backend/IProfile.h"

#include <string>

namespace Mlt
{
class Consumer;
}

namespace Backend
{
namespace MLT
{
class MLTProducer;

class MLTConsumer : public IConsumer, public MLTService
{
    public:
        MLTConsumer();
        MLTConsumer( IProfile& profile, const char *id, IConsumerEventCb* callback = nullptr );
        virtual ~MLTConsumer();

        static void     onConsumerStarted( void* owner, MLTConsumer* self );
        static void     onConsumerStopped( void* owner, MLTConsumer* self );

        virtual void    setName( const char* name ) override;
        virtual void    setCallback( IConsumerEventCb* callback ) override;

        virtual void    start() override;
        virtual void    stop() override;
        virtual bool    isStopped() const override;

        virtual int     volume() const override;
        virtual void    setVolume( int volume ) override;

        virtual bool    connect( IProducer& producer ) override;
        virtual bool    isConnected() const override;

    protected:
        Mlt::Consumer*      m_consumer;
        IConsumerEventCb*   m_callback;
        MLTProducer*        m_producer;
        std::string         m_name;
};

class MLTSdlConsumer : public MLTConsumer
{
    public:
        MLTSdlConsumer()
            : MLTConsumer( Backend::instance()->profile(), "sdl" ) { }

        void setWindowId( intptr_t id );
};

class MLTFFmpegConsumer : public MLTConsumer
{
    public:
        MLTFFmpegConsumer()
            : MLTConsumer( Backend::instance()->profile(), "avformat" ) { }

        void    setTarget( const char* path );
        void    setWidth( int width );
        void    setHeight( int height );
        void    setFrameRate( int num, int den );
        void    setAspectRatio( int num, int den );
        void    setVideoBitrate( int kbps );
        void    setAudioBitrate( int kbps );
        void    setChannels( int channels );
        void    setAudioSampleRate( int rate );

};

}
}

#endif // MLTCONSUMER_H
