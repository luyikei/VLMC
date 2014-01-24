/*****************************************************************************
 * VLCMedia.h: Binding for libvlc_media
 *****************************************************************************
 * Copyright (C) 2008-2010 VideoLAN
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

#ifndef VLCMEDIA_H
#define VLCMEDIA_H

#include "vlc/vlc.h"

#include <QString>

#include "VLCpp.hpp"

namespace LibVLCpp
{
    class   Media : public Internal< libvlc_media_t >
    {
    public:

        Media( const QString& filename );
        ~Media();
        void                addOption( const char* opt );
        void                addOption( const QString& opt );
        void                setVideoLockCallback( void* );
        void                setVideoUnlockCallback( void* );
        void                setAudioLockCallback( void* );
        void                setAudioUnlockCallback( void* );
        void                setVideoDataCtx( void* dataCtx );
        void                setAudioDataCtx( void* dataCtx );
        const QString&      getFileName() const;
        void                parse();
        void                fetchTrackInfo();
        unsigned int videoCodec() const;
        unsigned int audioCodec() const;

    private:
        QString                     m_fileName;
        libvlc_media_track_t        **m_tracks;
        // this has not to be equal to nb video tracks + nb audio tracks.
        // it is only meant to use when iterating over m_tracksInfo
        int                         m_nbTracks;
    };
}

#endif // VLCMEDIA_H
