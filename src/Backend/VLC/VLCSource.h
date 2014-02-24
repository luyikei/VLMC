/*****************************************************************************
 * VLCSource.h: Implementation of Source based on a libvlc_media_t
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

#ifndef VLCRSOURCE_H
#define VLCRSOURCE_H

#include <QImage>

#include "Backend/ISource.h"
#include "VLCMedia.h"

namespace Backend
{
namespace VLC
{

class VLCBackend;
class VmemRenderer;

class VLCSource : public ISource
{
public:
    VLCSource( VLCBackend* backend, const QString& path );
    virtual ~VLCSource();
    virtual ISourceRenderer*    createRenderer( ISourceRendererEventCb* callback );
    virtual bool                preparse();
    virtual bool                isParsed() const;
    virtual quint32             width() const;
    virtual quint32             height() const;
    virtual int64_t             length() const;
    virtual float               fps() const;
    virtual bool                hasVideo() const;
    virtual unsigned int        nbVideoTracks() const;
    virtual bool                hasAudio() const;
    virtual unsigned int        nbAudioTracks() const;
    const uint8_t*              snapshot() const;
    virtual int64_t             nbFrames() const;

    // Below this point are backend internal methods:
    LibVLCpp::Media*            media();

private:
    bool                        computeSnapshot( VmemRenderer* renderer );

private:
    VLCBackend*                 m_backend;
    LibVLCpp::Media*            m_media;
    unsigned int                m_width;
    unsigned int                m_height;
    float                       m_fps;
    unsigned int                m_nbVideoTracks;
    unsigned int                m_nbAudioTracks;
    int64_t                     m_length; //in milliseconds.
    QImage*                     m_snapshot;
    bool                        m_isParsed;
    int64_t                     m_nbFrames;
};


} //VLC
} //Backend

#endif // VLCRSOURCE_H
