/*****************************************************************************
 * EventWaiter.h: Helper to wait on a LibVLC event
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

#ifndef EVENTWAITER_H
#define EVENTWAITER_H

#include "VLCMediaPlayer.h"

namespace Backend
{
namespace VLC
{

class EventWaiter
{
public:
    /**
     * @brief initialize the EventWaiter
     *
     * This method will lock a mutex, preventing any further event processing.
     * Basic use scheme is
     * \li EventWaiter* ew = new EventWaiter( mediaPlayer );
     * \li ev->add( <event> );
     * \li <whatever action that will lead to an asynchronous event>
     * \li we->wait(...);
     *
     * @warning If the event comes synchronously, this will deadlock. But if
     *          it happens synchronously, why would you need this at all?
     */
    EventWaiter( LibVLCpp::MediaPlayer* mediaPlayer );
    ~EventWaiter();

    enum    Result
    {
        Success, ///The event has been emited.
        Canceled,///A cancelation event has been emited first.
        Timeout  ///Timeout has been reached.
    };
    void            add( libvlc_event_type_t event );
    Result          wait( unsigned long timeoutMs );

    typedef bool    (*ValidationCallback)( const libvlc_event_t* event );
    void            setValidationCallback( ValidationCallback callback );

private:
    static void     eventsCallback( const libvlc_event_t *event, void* data );

private:
    LibVLCpp::MediaPlayer*      m_mediaPlayer;
    QList<libvlc_event_type_t>  m_events;
    QWaitCondition              m_waitCond;
    QMutex                      m_mutex;
    ValidationCallback          m_validationCallback;
    bool                        m_found;
};

} //VLC
} //Backend
#endif // EVENTWAITER_H
