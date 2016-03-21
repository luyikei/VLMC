/*****************************************************************************
 * VLCVmemRenderer.cpp: Private VLC backend implementation of a renderer using vmem
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

#include <QImage>

#include "VLCVmemRenderer.h"
#include "VLCSource.h"
#include "Tools/VlmcDebug.h"


using namespace Backend::VLC;

VmemRenderer::VmemRenderer( VLCBackend* backend, VLCSource *source , ISourceRendererEventCb *callback )
    : VLCSourceRenderer( backend, source, callback )
    , m_snapshotRequired( false )
{
    m_media.parse();
    setName( "VmemRenderer" );
    m_snapshot = new QImage( 320, 180, QImage::Format_RGB32 );
    m_mediaPlayer.setVideoFormat( "RV32", m_snapshot->width(), m_snapshot->height(), m_snapshot->bytesPerLine() );
    m_mediaPlayer.setVideoCallbacks(
        // Lock:
        [this]( void** planes ) {
            return vmemLock( planes );
        },
        // Unlock:
        nullptr,
        // Display:
        [this]( void* picture ) {
            vmemUnlock( picture );
        }
    );
    if ( m_mediaPlayer.setAudioOutput( "dummy" ) == false )
        vlmcWarning() << "Failed to disable audio output";
}

VmemRenderer::~VmemRenderer()
{
    /*
     * We need to stop the media player from here, otherwise m_mutex would be
     * destroyed in a potentially locked state, while the vmem tries to lock/unlock.
     */
    stop();
    delete m_snapshot;
}

::VLC::MediaPlayer&
VmemRenderer::mediaPlayer()
{
    return m_mediaPlayer;
}

QImage*
VmemRenderer::waitSnapshot()
{
    QMutexLocker lock( &m_mutex );
    m_snapshotRequired = true;
    if ( m_waitCond.wait( &m_mutex, 3000 ) == false )
        return NULL;
    return new QImage( *m_snapshot );
}

void*
VmemRenderer::vmemLock( void **planes)
{
    QMutexLocker lock( &m_mutex );
    *planes = m_snapshot->bits();
    return m_snapshot;
}

void
VmemRenderer::vmemUnlock( void* picture )
{
    QMutexLocker lock( &m_mutex );
    Q_UNUSED( picture );
    if ( m_snapshotRequired == true )
    {
        m_snapshotRequired = false;
        m_waitCond.wakeAll();
    }
}
