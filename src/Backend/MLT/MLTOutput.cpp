/*****************************************************************************
 * MLTOutput.cpp:  Wrapper of Mlt::Consumer
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "MLTOutput.h"
#include "MLTInput.h"
#include "MLTProfile.h"
#include "MLTBackend.h"

#include <mlt++/MltProducer.h>
#include <mlt++/MltConsumer.h>
#include <mlt++/MltProfile.h>

#include <cassert>

using namespace Backend::MLT;

MLTOutput::MLTOutput()
    : MLTOutput( Backend::instance()->profile(), "sdl" )
{

}

MLTOutput::MLTOutput( Backend::IProfile& profile, const char *id, Backend::IOutputEventCb* callback )
    : m_callback( callback )
    , m_input( nullptr )
{
    MLTProfile& mltProfile = static_cast<MLTProfile&>( profile );
    m_consumer = new Mlt::Consumer( *mltProfile.m_profile, id );
    if ( isValid() == false )
        throw InvalidServiceException();
}

MLTOutput::~MLTOutput()
{
    stop();
    delete m_consumer;
}

Mlt::Consumer*
MLTOutput::consumer()
{
    return m_consumer;
}

Mlt::Consumer*
MLTOutput::consumer() const
{
    return m_consumer;
}

Mlt::Service*
MLTOutput::service()
{
    return consumer();
}

Mlt::Service*
MLTOutput::service() const
{
    return consumer();
}

void
MLTOutput::onOutputStarted( void*, MLTOutput* self )
{
    if ( self->m_callback == nullptr )
        return;
    self->m_callback->onPlaying();
}

void
MLTOutput::onOutputStopped( void*, MLTOutput* self )
{
    if ( self->m_callback == nullptr )
        return;
    self->m_callback->onStopped();
}

void
MLTOutput::setName( const char* name )
{
    m_name = std::string( name );
}

void
MLTOutput::setCallback(Backend::IOutputEventCb *callback)
{
    if ( callback == nullptr )
        return;
    m_callback = callback;
    consumer()->listen( "consumer-thread-started", this, (mlt_listener)MLTOutput::onOutputStarted );
    consumer()->listen( "consumer-stopped", this, (mlt_listener)MLTOutput::onOutputStopped );
}

void
MLTOutput::start()
{
    consumer()->start();
}

void
MLTOutput::stop()
{
    consumer()->stop();
}

bool
MLTOutput::isStopped() const
{
    return consumer()->is_stopped();
}

int
MLTOutput::volume() const
{
    return consumer()->get_double( "volume" ) * 100;
}

void
MLTOutput::setVolume( int volume )
{
    consumer()->set( "volume", volume / 100.f );
}

bool
MLTOutput::connect( Backend::IInput& input )
{
    MLTInput* mltInput = dynamic_cast<MLTInput*>( &input );
    assert( mltInput );
    m_input = mltInput;
    return !consumer()->connect( *(mltInput->producer()) );
}

bool
MLTOutput::isConnected() const
{
    return m_input != nullptr;
}

void
MLTSdlOutput::setWindowId( intptr_t id )
{
    consumer()->set( "window_id", std::to_string( id ).c_str() );
}

void
MLTFFmpegOutput::setTarget( const char* path )
{
    consumer()->set( "target", path );
}

void
MLTFFmpegOutput::setWidth( int width )
{
    consumer()->set( "width", width );
}

void
MLTFFmpegOutput::setHeight( int height )
{
    consumer()->set( "height", height );
}

void
MLTFFmpegOutput::setFrameRate( int num, int den )
{
    consumer()->set( "frame_rate_num", num );
    consumer()->set( "frame_rate_den", den );
}

void
MLTFFmpegOutput::setAspectRatio( int num, int den )
{
    consumer()->set( "display_aspect_num", num );
    consumer()->set( "display_aspect_den", den );
}

void
MLTFFmpegOutput::setVideoBitrate( int kbps )
{
    std::string str = std::to_string( kbps ) + "K";
    consumer()->set( "vb", str.c_str() );
}

void
MLTFFmpegOutput::setAudioBitrate( int kbps )
{
    std::string str = std::to_string( kbps ) + "K";
    consumer()->set( "ab", str.c_str() );
}

void
MLTFFmpegOutput::setChannels( int channels )
{
    consumer()->set( "channels", channels );
}

void
MLTFFmpegOutput::setAudioSampleRate( int rate )
{
    consumer()->set( "frequency", rate );
}
