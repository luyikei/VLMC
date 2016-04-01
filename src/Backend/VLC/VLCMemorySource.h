/*****************************************************************************
 * VLCMemorySource.cpp: Implementation of ISource based on a libvlc_media_t using imem
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

#ifndef VLCMEMORYSOURCE_H
#define VLCMEMORYSOURCE_H

#include "Backend/ISource.h"

#include <QString>

namespace LibVLCpp
{
    class Media;
}

namespace Backend
{
namespace VLC
{

class VLCBackend;

class VLCMemorySource : public IMemorySource
{
public:
    VLCMemorySource( VLCBackend* backend );
    ~VLCMemorySource();
    virtual void                setWidth( unsigned int width );
    virtual void                setHeight( unsigned int height );
    virtual void                setFps( float fps );
    virtual void                setAspectRatio( const char* aspectRatio );
    virtual void                setNumberChannels( unsigned int nbChannels );
    virtual void                setSampleRate( unsigned int sampleRate );
    virtual ISourceRenderer*    createRenderer( ISourceRendererEventCb* callback );

    // Below this point are backend internal methods:
    LibVLCpp::Media*            media();
    unsigned int                width() const;
    unsigned int                height() const;
    float                       fps() const;
    const QString&              aspectRatio() const;
    unsigned int                numberChannels() const;
    unsigned int                sampleRate() const;

private:
    VLCBackend*                 m_backend;
    LibVLCpp::Media*            m_media;
    unsigned int                m_width;
    unsigned int                m_height;
    float                       m_fps;
    QString                     m_aspectRatio;
    unsigned int                m_nbChannels;
    unsigned int                m_sampleRate;
};

} // VLC
} // Backend

#endif // VLCMEMORYSOURCE_H
