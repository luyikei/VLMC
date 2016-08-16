/*****************************************************************************
 * MLTInput.cpp:  Wrapper of Mlt::Producer
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

#include "MLTInput.h"
#include "MLTProfile.h"
#include "MLTBackend.h"
#include "MLTFilter.h"

#include <mlt++/MltFrame.h>
#include <mlt++/MltFilter.h>
#include <mlt++/MltProducer.h>
#include <cstring>
#include <cassert>

using namespace Backend::MLT;

MLTInput::MLTInput()
    : m_producer( nullptr )
    , m_callback( nullptr )
    , m_paused( false )
    , m_nbVideoTracks( 0 )
    , m_nbAudioTracks( 0 )
{

}

void
MLTInput::calcTracks()
{
    char s[70];
    int  nbStreams = producer()->get_int( "meta.media.nb_streams" );
    for ( int i = 0; i < nbStreams; ++i )
    {
        sprintf( s, "meta.media.%d.stream.type", i );
        auto type = producer()->get( s );

        if ( type == nullptr )
            continue;

        if ( strcmp( type, "video" ) == 0 )
            m_nbVideoTracks++;
        else if ( strcmp( type, "audio" ) == 0 )
            m_nbAudioTracks++;
    }
}

MLTInput::MLTInput( Mlt::Producer* producer, IInputEventCb* callback )
    : MLTInput()
{
    m_producer = producer;
    setCallback( callback );
    calcTracks();
    if ( isValid() == false )
        throw InvalidServiceException();
}

MLTInput::MLTInput( IProfile& profile, const char* path, IInputEventCb* callback )
    : MLTInput()
{
    std::string temp = std::string( "avformat:" ) + path;
    MLTProfile& mltProfile = static_cast<MLTProfile&>( profile );
    m_producer = new Mlt::Producer( *mltProfile.m_profile, "loader", temp.c_str() );
    setCallback( callback );
    calcTracks();
    if ( isValid() == false )
        throw InvalidServiceException();
}

MLTInput::MLTInput( const char* path, IInputEventCb* callback )
    : MLTInput( Backend::instance()->profile(), path, callback )
{
}


MLTInput::~MLTInput()
{
    delete m_producer;
}

Mlt::Producer*
MLTInput::producer()
{
    return m_producer;
}

Mlt::Producer*
MLTInput::producer() const
{
    return m_producer;
}

Mlt::Service*
MLTInput::service()
{
    return producer();
}

Mlt::Service*
MLTInput::service() const
{
    return producer();
}

void
MLTInput::onPropertyChanged( void*, MLTInput* self, const char* id )
{
    if ( self == nullptr || self->m_callback == nullptr )
        return;

    if ( strcmp( id, "_position" ) == 0 )
    {
        self->m_callback->onPositionChanged( self->position() );
        if ( self->position() >= self->playableLength() - 1 )
            self->m_callback->onEndReached();
    }
    else if ( strcmp( id, "length" ) == 0 )
        self->m_callback->onLengthChanged( self->playableLength() );

}

void
MLTInput::setCallback( Backend::IInputEventCb* callback )
{
    if ( callback == nullptr )
        return;

    m_callback = callback;
    producer()->listen( "property-changed", this, (mlt_listener)MLTInput::onPropertyChanged );
}

const char*
MLTInput::path() const
{
    return producer()->get( "resource" );
}

int64_t
MLTInput::begin() const
{
    return producer()->get_in();
}

int64_t
MLTInput::end() const
{
    return producer()->get_out();
}

void
MLTInput::setBegin( int64_t begin )
{
    producer()->set( "in", (int)begin );
}

void
MLTInput::setEnd( int64_t end )
{
    producer()->set( "out", (int)end );
}

void
MLTInput::setBoundaries( int64_t begin, int64_t end )
{
    if ( end == EndOfParent )
        // parent() will be producer() itself if it has no parent
        end = producer()->parent().get_out();
    producer()->set_in_and_out( begin, end );
}

std::unique_ptr<Backend::IInput>
MLTInput::cut( int64_t begin, int64_t end )
{
    return std::unique_ptr<IInput>( new MLTInput( producer()->cut( begin, end ) ) );
}

bool
MLTInput::isCut() const
{
    return producer()->is_cut();
}

bool
MLTInput::sameClip( Backend::IInput& that ) const
{
    MLTInput* input = dynamic_cast<MLTInput*>( &that );
    assert( input );

    return producer()->same_clip( *input->producer() );
}

bool
MLTInput::runsInto( Backend::IInput& that ) const
{
    MLTInput* input = dynamic_cast<MLTInput*>( &that );
    assert( input );

    return producer()->runs_into( *input->producer() );
}

int64_t
MLTInput::playableLength() const
{
    return producer()->get_playtime();
}

int64_t
MLTInput::length() const
{
    return producer()->get_length();
}

const char*
MLTInput::lengthTime() const
{
    return producer()->get_length_time( mlt_time_clock );
}

int64_t
MLTInput::position() const
{
    return producer()->position();
}

void
MLTInput::setPosition( int64_t position )
{
    producer()->seek( position );
}

int64_t
MLTInput::frame() const
{
    return producer()->frame();
}

uint8_t*
MLTInput::waveform( uint32_t width, uint32_t height ) const
{
    std::unique_ptr<Mlt::Frame> waveformFrame( producer()->get_frame() );
    return waveformFrame->get_waveform( (int)width, (int)height );
}

uint8_t*
MLTInput::image( uint32_t width, uint32_t height ) const
{
    std::unique_ptr<Mlt::Frame> imageFrame( producer()->get_frame() );
    return imageFrame->fetch_image( mlt_image_rgb24a, (int)width, (int)height );
}

double
MLTInput::fps() const
{
    return producer()->get_fps();
}

double
MLTInput::aspectRatio() const
{
    return producer()->get_double( "aspect_ratio" );
}

int
MLTInput::width() const
{
    // FIXME: Sometimes I can't get width and height
    auto v = producer()->get_int( "width" );
    return v > 0 ? v : Backend::instance()->profile().width();
}

int
MLTInput::height() const
{
    auto v = producer()->get_int( "height" );
    return v > 0 ? v : Backend::instance()->profile().height();
}

bool
MLTInput::hasVideo() const
{
    return m_nbVideoTracks > 0;
}

int
MLTInput::nbVideoTracks() const
{
    return m_nbVideoTracks;
}

bool
MLTInput::hasAudio() const
{
    return m_nbAudioTracks > 0;
}

int
MLTInput::nbAudioTracks() const
{
    return m_nbAudioTracks;
}

void
MLTInput::playPause()
{
    if ( m_paused )
        producer()->set_speed( 1.0 );
    else
        producer()->set_speed( 0.0 );
    m_paused = !m_paused;

    if ( m_callback )
    {
        if ( m_paused == true )
            m_callback->onPaused();
        else
            m_callback->onPlaying();
    }
}

bool
MLTInput::isPaused() const
{
    return m_paused;
}

void
MLTInput::setPause( bool isPaused )
{
    // It will be negated again in playPause.
    m_paused = !isPaused;
    playPause();
}

void
MLTInput::nextFrame()
{
    if ( producer()->position() < producer()->get_out() )
    {
        if ( isPaused() == false )
            playPause();
        producer()->seek( producer()->position() + 1 );
    }
}

void
MLTInput::previousFrame()
{
    if ( producer()->get_in() < producer()->position() )
    {
        if ( isPaused() == false )
            playPause();
        producer()->seek( producer()->position() - 1 );
    }
}

bool
MLTInput::isBlank() const
{
    return producer()->is_blank();
}


bool
MLTInput::attach( Backend::IFilter& filter )
{
    MLTFilter* mltFilter = dynamic_cast<MLTFilter*>( &filter );
    assert( mltFilter );
    auto ret = producer()->attach( *mltFilter->filter() );
    mltFilter->connect( *this );
    return !ret;
}

bool
MLTInput::detach( Backend::IFilter& filter )
{
    MLTFilter* mltFilter = dynamic_cast<MLTFilter*>( &filter );
    assert( mltFilter );
    return !producer()->detach( *mltFilter->filter() );
}

bool
MLTInput::detach( int index )
{
    auto filter = producer()->filter( index );
    auto ret = producer()->detach( *filter );
    delete filter;
    return !ret;
}

int
MLTInput::filterCount() const
{
    return producer()->filter_count();
}

bool
MLTInput::moveFilter( int from, int to )
{
    return !producer()->move_filter( from, to );
}

std::shared_ptr<Backend::IFilter>
MLTInput::filter( int index ) const
{
    return std::shared_ptr<Backend::IFilter>( new MLTFilter( producer()->filter( index ), producer() ) );
}
