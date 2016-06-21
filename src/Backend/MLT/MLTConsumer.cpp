/*****************************************************************************
 * MLTConsumer.cpp:  Wrapper of Mlt::Consumer
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

#include "MLTConsumer.h"
#include "MLTProducer.h"
#include "MLTProfile.h"
#include "MLTBackend.h"

#include <mlt++/MltProducer.h>
#include <mlt++/MltConsumer.h>
#include <mlt++/MltProfile.h>

#include <cassert>

using namespace Backend::MLT;

MLTConsumer::MLTConsumer()
    : MLTConsumer( Backend::instance()->profile(), "sdl" )
{

}

MLTConsumer::MLTConsumer( Backend::IProfile& profile, const char *id, Backend::IConsumerEventCb* callback )
    : m_callback( callback )
    , m_producer( nullptr )
{
    MLTProfile& mltProfile = static_cast<MLTProfile&>( profile );
    m_consumer = new Mlt::Consumer( *mltProfile.m_profile, id );
    m_service = m_consumer;
}

MLTConsumer::~MLTConsumer()
{
    stop();
    delete m_consumer;
}

void
MLTConsumer::onConsumerStarted( void*, MLTConsumer* self )
{
    if ( self->m_callback == nullptr )
        return;
    self->m_callback->onPlaying();
}

void
MLTConsumer::onConsumerStopped( void*, MLTConsumer* self )
{
    if ( self->m_callback == nullptr )
        return;
    self->m_callback->onStopped();
}

void
MLTConsumer::setName( const char* name )
{
    m_name = std::string( name );
}

void
MLTConsumer::setCallback(Backend::IConsumerEventCb *callback)
{
    if ( callback == nullptr )
        return;
    m_callback = callback;
    m_consumer->listen( "consumer-thread-started", this, (mlt_listener)MLTConsumer::onConsumerStarted );
    m_consumer->listen( "consumer-stopped", this, (mlt_listener)MLTConsumer::onConsumerStopped );
}

void
MLTConsumer::start()
{
    m_consumer->start();
}

void
MLTConsumer::stop()
{
    m_consumer->stop();
}

bool
MLTConsumer::isStopped() const
{
    return m_consumer->is_stopped();
}

int
MLTConsumer::volume() const
{
    return m_consumer->get_double( "volume" ) * 100;
}

void
MLTConsumer::setVolume( int volume )
{
    m_consumer->set( "volume", volume / 100.f );
}

bool
MLTConsumer::connect( Backend::IProducer& producer )
{
    MLTProducer* mltProducer = dynamic_cast<MLTProducer*>( &producer );
    assert( mltProducer );
    m_producer = mltProducer;
    return m_consumer->connect( *(mltProducer->m_producer) );
}

bool
MLTConsumer::isConnected() const
{
    return m_producer != nullptr;
}

void
MLTSdlConsumer::setWindowId( intptr_t id )
{
    m_consumer->set( "window_id", std::to_string( id ).c_str() );
}

void
MLTFFmpegConsumer::setTarget( const char* path )
{
    m_consumer->set( "target", path );
}

void
MLTFFmpegConsumer::setWidth( int width )
{
    m_consumer->set( "width", width );
}

void
MLTFFmpegConsumer::setHeight( int height )
{
    m_consumer->set( "height", height );
}

void
MLTFFmpegConsumer::setFrameRate( int num, int den )
{
    m_consumer->set( "frame_rate_num", num );
    m_consumer->set( "frame_rate_den", den );
}

void
MLTFFmpegConsumer::setAspectRatio( int num, int den )
{
    m_consumer->set( "display_aspect_num", num );
    m_consumer->set( "display_aspect_den", den );
}

void
MLTFFmpegConsumer::setVideoBitrate( int kbps )
{
    std::string str = std::to_string( kbps ) + "K";
    m_consumer->set( "vb", str.c_str() );
}

void
MLTFFmpegConsumer::setAudioBitrate( int kbps )
{
    std::string str = std::to_string( kbps ) + "K";
    m_consumer->set( "ab", str.c_str() );
}

void
MLTFFmpegConsumer::setChannels( int channels )
{
    m_consumer->set( "channels", channels );
}

void
MLTFFmpegConsumer::setAudioSampleRate( int rate )
{
    m_consumer->set( "frequency", rate );
}
