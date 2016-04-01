/*****************************************************************************
 * VmemRenderer.h: Private VLC backend implementation of a renderer using vmem
 *****************************************************************************
 * Copyright (C) 2008-2016 VideoLAN
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

#ifndef VLCVMEMRENDERER_H
#define VLCVMEMRENDERER_H

#include <QMutex>
#include <QWaitCondition>

#include "VLCSourceRenderer.h"

class QImage;

namespace Backend
{
namespace VLC
{

class VLCSource;

class VmemRenderer : public VLCSourceRenderer
{
public:
    VmemRenderer(VLCBackend *backend, VLCSource* source, ISourceRendererEventCb* callback );
    virtual ~VmemRenderer();
    ::VLC::MediaPlayer&  mediaPlayer();
    /**
     * @brief waitSnapshot  Wait for a snapshot to be computed and returns it.
     *  The renderer doesn't own the snapshot, and it will have to be released by
     *  the caller.
     * @return
     */
    QImage *waitSnapshot();

private:
    void*    vmemLock( void **planes );
    void     vmemUnlock( void* picture );

private:
    QImage*         m_snapshot;
    bool            m_snapshotRequired;
    QMutex          m_mutex;
    QWaitCondition  m_waitCond;
};

}
}

#endif // VMEMRENDERER_H
