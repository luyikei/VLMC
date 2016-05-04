/*****************************************************************************
 * ClipSmemRenderer.cpp: Render from a Clip
 *****************************************************************************
 * Copyright (C) 2008-2016 VideoLAN
 *
 * Authors: Hugo Beauzée-Luyssen <hugo@beauzee.fr>
 *          Yikei Lu <luyikei.qmltu@gmail.com>
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

#ifndef CLIPSMEMRENDERER_H
#define CLIPSMEMRENDERER_H

class QMutex;
class QWaitCondition;

#include <QObject>
#include <QQueue>

#include "Workflow/ClipHelper.h"
#include "Workflow/Types.h"
#include "Tools/RendererEventWatcher.h"

class ClipSmemRenderer : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY( ClipSmemRenderer )

public:

    /**
     *  \brief  Used to know which way you want to get a computed output.
     *          Pop: the buffer is popped and returned
     *          Get: the buffer is just returned (for paused mode for instance)
     */
    enum        GetMode
    {
        Pop,
        Get,
    };

    explicit ClipSmemRenderer( ClipHelper* ch, quint32 width = 0, quint32 height = 0, bool fullSpeedRender = false );
    ~ClipSmemRenderer();

    Workflow::Frame*            getOutput( Workflow::TrackType trackType, GetMode mode, qint64 currentFrame );
    void                        stop();
    void                        start();
    void                        setPause( bool val );
    void                        setTime( int64_t time );

    void                        releasePrealocated();
    void                        flushComputedBuffers();

    RendererEventWatcher*       eventWatcher();

private:
    static constexpr qint32     maxNumBuffers( Workflow::TrackType trackType );

    Backend::ISourceRenderer*   m_renderer;
    RendererEventWatcher*       m_eventWatcher;
    QMutex*                     m_renderLock[Workflow::NbTrackType];
    QWaitCondition*             m_renderWaitCond[Workflow::NbTrackType];

    QQueue<Workflow::Frame*>    m_availableBuffers[Workflow::NbTrackType];
    QQueue<Workflow::Frame*>    m_computedBuffers[Workflow::NbTrackType];
    Workflow::Frame*            m_lastReturnedBuffer[Workflow::NbTrackType];

    quint32                     m_width;
    quint32                     m_height;

    void                        preallocate();
    void                        pause();

    static void                 audioLock( void *data,quint8** pcm_buffer , size_t size );
    static void                 audioUnlock( void *data,
                                             uint8_t * pcm_buffer, unsigned int channels,
                                             unsigned int rate, unsigned int nb_samples,
                                             unsigned int bits_per_sample,
                                             size_t size, int64_t pts );
    static void                 videoLock( void* data, uint8_t** p_buffer, size_t size );
    static void                 videoUnlock( void *data, uint8_t* buffer, int width,
                                             int height, int bpp, size_t size, int64_t pts );
signals:
    void                        bufferReachedMax();

};

#endif // CLIPSMEMRENDERER_H
