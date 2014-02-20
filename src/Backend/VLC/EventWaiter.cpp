/*****************************************************************************
 * EventWaiter.cpp: Helper to wait on a LibVLC event
 *****************************************************************************
 * Copyright (C) 2008-2014 VideoLAN
 *
 * Authors: Hugo Beauzée-Luyssen <hugo@beauzee.fr>
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

#include "EventWaiter.h"

using namespace Backend::VLC;

EventWaiter::EventWaiter( LibVLCpp::MediaPlayer* mediaPlayer )
    : m_mediaPlayer( mediaPlayer )
    , m_validationCallback( NULL )
    , m_found( true )
{
    m_mediaPlayer->registerEvents( &EventWaiter::eventsCallback, this );
    m_mutex.lock();
}

EventWaiter::~EventWaiter()
{
    m_mediaPlayer->unregisterEvents( &EventWaiter::eventsCallback, this );
}

void EventWaiter::add(libvlc_event_type_t event)
{
    m_events.push_back( event );
}

EventWaiter::Result
EventWaiter::wait(unsigned long timeoutMs)
{
    if ( m_waitCond.wait( &m_mutex, timeoutMs ) == false )
    {
        m_mutex.unlock();
        return Timeout;
    }
    m_mutex.unlock();
    return m_found == true ? Success : Canceled;
}

void
EventWaiter::setValidationCallback(EventWaiter::ValidationCallback callback)
{
    m_validationCallback = callback;
}

void EventWaiter::eventsCallback( const libvlc_event_t* event, void *data )
{
    EventWaiter* self = reinterpret_cast<EventWaiter*>( data );
    QMutexLocker lock( &self->m_mutex );

    if ( self->m_events.contains( event->type ) == true )
    {
        if ( self->m_validationCallback != NULL && self->m_validationCallback( event ) == false )
            return;
        self->m_found = true;
        self->m_waitCond.wakeAll();
        return;
    }

    if ( event->type == libvlc_MediaPlayerEncounteredError ||
         event->type == libvlc_MediaPlayerEndReached )
    {
        self->m_waitCond.wakeAll();
    }
}

