/*****************************************************************************
 * MetaDataWorker.h: MetaDataWorker
 *****************************************************************************
 * Copyright (C) 2008-2010 VideoLAN
 *
 * Authors: Hugo Beauzée-Luyssen <hugo@beauzee.fr>
 * Authors: Geoffroy Lacarriere <geoffroylaca@gmail.com>
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

#ifndef METADATAWORKER_H
#define METADATAWORKER_H

#include "config.h"
#include "Media.h"

#include <QList>
#include <QTemporaryFile>
#include <QTime>
#include <QThread>

class   QTimer;

namespace LibVLCpp
{
    class   MediaPlayer;
}

class MetaDataWorker : public QThread
{
    Q_OBJECT
    Q_DISABLE_COPY( MetaDataWorker )

    public:
        MetaDataWorker( LibVLCpp::MediaPlayer* mediaPlayer, Media* media );
        ~MetaDataWorker();
        void                        run();

    private:
//        void                        prepareAudioSpectrumComputing();
//        void                        addAudioValue( int value );

    private:
        void                        metaDataAvailable();
        static void                 vmemSetFormat( void* data, char* chroma, unsigned int* width,
                                                   unsigned int* height, unsigned int* pitches, unsigned int* lines );
        static void*                vmemLock( void* data, void **planes );
//        static void                 lock( MetaDataWorker* metaDataWorker, quint8** pcm_buffer , unsigned int size );
//        static void                 unlock( MetaDataWorker* metaDataWorker, quint8* pcm_buffer,
//                                        unsigned int channels, unsigned int rate,
//                                        unsigned int nb_samples, unsigned int bits_per_sample,
//                                        unsigned int size, int pts );
#ifdef WITH_GUI
        void    computeSnapshot();
#endif

    private:
        LibVLCpp::MediaPlayer*      m_mediaPlayer;
        Media*                      m_media;
        QImage*                     m_snapshot;
        unsigned char*              m_audioBuffer;

//    private slots:
//        void    generateAudioSpectrum();

    signals:
        void    computed();
        /**
         * @brief snapshotReady Signals that a media now has a snapshot available
         * @warning             This gives the ownership of the snapshot to the recipient.
         *                      Memory will *NOT* be released by the MetaDataWorker
         */
        void    snapshotReady( const QImage* snapshot );
        void    failed( Media* media );
};

#endif // METADATAWORKER_H
