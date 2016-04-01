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

#include "VLCMemorySource.h"
#include "VLCSourceRenderer.h"

using namespace Backend;
using namespace Backend::VLC;

VLCMemorySource::VLCMemorySource( VLCBackend *backend )
    : m_backend( backend )
{
}

VLCMemorySource::~VLCMemorySource()
{
}

void
VLCMemorySource::setWidth(unsigned int width)
{
    m_width = width;
}

void
VLCMemorySource::setHeight(unsigned int height)
{
    m_height = height;
}

void
VLCMemorySource::setFps(float fps)
{
    m_fps = fps;
}

void
VLCMemorySource::setAspectRatio(const char *aspectRatio)
{
    m_aspectRatio = aspectRatio;
}

void
VLCMemorySource::setNumberChannels(unsigned int nbChannels)
{
    m_nbChannels = nbChannels;
}

void
VLCMemorySource::setSampleRate(unsigned int sampleRate)
{
    m_sampleRate = sampleRate;
}

ISourceRenderer*
VLCMemorySource::createRenderer( ISourceRendererEventCb *callback )
{
    return new VLCSourceRenderer( m_backend, this, callback );
}

unsigned int
VLCMemorySource::width() const
{
    return m_width;
}

unsigned int
VLCMemorySource::height() const
{
    return m_height;
}

float
VLCMemorySource::fps() const
{
    return m_fps;
}

const QString&
VLCMemorySource::aspectRatio() const
{
    return m_aspectRatio;
}

unsigned int
VLCMemorySource::numberChannels() const
{
    return m_nbChannels;
}

unsigned int
VLCMemorySource::sampleRate() const
{
    return m_sampleRate;
}
