/*****************************************************************************
 * MLTOutput.h:  Wrapper of Mlt::Consumer
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

#ifndef MLTOUTPUT_H
#define MLTOUTPUT_H

#include "MLTService.h"
#include "Backend/IOutput.h"
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
class MLTInput;

class MLTOutput : public IOutput, public MLTService
{
    public:
        MLTOutput();
        MLTOutput( IProfile& profile, const char *id, IOutputEventCb* callback = nullptr );
        virtual ~MLTOutput();

        virtual Mlt::Consumer*  consumer();
        virtual Mlt::Consumer*  consumer() const;

        virtual Mlt::Service*   service() override;
        virtual Mlt::Service*   service() const override;

        static void     onOutputStarted( void* owner, MLTOutput* self );
        static void     onOutputStopped( void* owner, MLTOutput* self );

        virtual void    setName( const char* name ) override;
        virtual void    setCallback( IOutputEventCb* callback ) override;

        virtual void    start() override;
        virtual void    stop() override;
        virtual bool    isStopped() const override;

        virtual int     volume() const override;
        virtual void    setVolume( int volume ) override;

        virtual bool    connect( IInput& input ) override;
        virtual bool    isConnected() const override;

    private:
        Mlt::Consumer*      m_consumer;
        IOutputEventCb*     m_callback;
        MLTInput*           m_input;
        std::string         m_name;
};

class MLTSdlOutput : public MLTOutput
{
    public:
        MLTSdlOutput()
            : MLTOutput( Backend::instance()->profile(), "sdl" ) { }

        void setWindowId( intptr_t id );
};

class MLTFFmpegOutput : public MLTOutput
{
    public:
        MLTFFmpegOutput()
            : MLTOutput( Backend::instance()->profile(), "avformat" ) { }

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

#endif // MLTOUTPUT_H
