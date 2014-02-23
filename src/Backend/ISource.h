/*****************************************************************************
 * ISource: Describes a source being rendered by a ISourceRenderer
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

#ifndef ISOURCE_HPP
#define ISOURCE_HPP

#include <stdint.h>

namespace Backend
{
    class ISourceRendererEventCb;
    class ISourceRenderer;

    class ISource
    {
        public:
            virtual ~ISource() {}
            virtual ISourceRenderer*    createRenderer( ISourceRendererEventCb* callback ) = 0;
            /**
             * @brief preparse  Parse this source for its information.
             *                  This method will block until computing is finished.
             * @return
             */
            virtual bool            preparse() = 0;
            virtual bool            isParsed() const = 0;
            virtual unsigned int    width() const = 0;
            virtual unsigned int    height() const = 0;
            virtual int64_t         length() const = 0;
            virtual float           fps() const = 0;
            virtual bool            hasVideo() const = 0;
            virtual unsigned int    nbVideoTracks() const = 0;
            virtual bool            hasAudio() const = 0;
            virtual unsigned int    nbAudioTracks() const = 0;
    };

    class IMemorySource
    {
        public:
            virtual ~IMemorySource(){}
            virtual void                setWidth( unsigned int width ) = 0;
            virtual void                setHeight( unsigned int height ) = 0;
            virtual void                setFps( float fps ) = 0;
            virtual void                setAspectRatio( const char* aspectRatio ) = 0;
            virtual void                setNumberChannels( unsigned int nbChannels ) = 0;
            virtual void                setSampleRate( unsigned int sampleRate ) = 0;
            virtual ISourceRenderer*    createRenderer( ISourceRendererEventCb* callback ) = 0;
    };
}

#endif // ISOURCE_HPP
