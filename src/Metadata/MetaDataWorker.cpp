/*****************************************************************************
 * MetaDataWorker.cpp: Implement the thread that will get the media informations
 *****************************************************************************
 * Copyright (C) 2008-2010 VideoLAN
 *
 * Authors: Hugo Beauzee-Luyssen <hugo@vlmc.org>
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

#include <QtDebug>

#include "vlmc.h"
#include "MetaDataWorker.h"
#include "Library.h"
#include "SettingsManager.h"
#include "VLCMediaPlayer.h"
#include "VLCMedia.h"
#include "Clip.h"

#ifdef WITH_GUI
# include <QPainter>
# include <QLabel>
# include <QImage>
#endif

#include <QTimer>

MetaDataWorker::MetaDataWorker( LibVLCpp::MediaPlayer* mediaPlayer, Media* media ) :
        m_mediaPlayer( mediaPlayer ),
        m_media( media ),
        m_audioBuffer( NULL )
{
    connect( this, SIGNAL( finished() ), this, SLOT( deleteLater() ) );
}

MetaDataWorker::~MetaDataWorker()
{
    if ( m_audioBuffer )
        delete m_audioBuffer;
}

static bool checkEvent(const LibVLCpp::MediaPlayer*, const libvlc_event_t* event)
{
    return event->type == libvlc_MediaPlayerTimeChanged &&
            event->u.media_player_time_changed.new_time > 0;
}

void
MetaDataWorker::run()
{
    QList<int>  cancel;

    cancel << libvlc_MediaPlayerEncounteredError << libvlc_MediaPlayerEndReached;

    m_mediaPlayer->configureWaitForEvent( libvlc_MediaPlayerTimeChanged, cancel, &checkEvent );

    m_media->addConstantParam( ":vout=dummy" );
    // In VLC 2.x we can't set the volume before the playback has started
    // so just switch off the audio-output in any case.
    m_mediaPlayer->setAudioOutput( "dummy" );
    m_mediaPlayer->setMedia( m_media->vlcMedia() );

    m_mediaPlayer->play();
    LibVLCpp::MediaPlayer::EventWaitResult res = m_mediaPlayer->waitForEvent( 3000 );
    if ( res != LibVLCpp::MediaPlayer::Success )
    {
        qWarning() << "Got" << (res == LibVLCpp::MediaPlayer::Timeout ? "timeout" : "failure")
                      << "while launching metadata processing";
        emit failed( m_media );
    }
    else
        metaDataAvailable();
}

void
MetaDataWorker::metaDataAvailable()
{
    m_media->setNbAudioTrack( m_mediaPlayer->getNbAudioTrack() );
    m_media->setNbVideoTrack( m_mediaPlayer->getNbVideoTrack() );

    Q_ASSERT_X( m_media->hasAudioTrack() == true || m_media->hasVideoTrack() == true,
                "metadata parsing", "Position can't be non 0 if no track is available" );

    //Don't be fooled by the extension, and probe the file for it's actual type:
    if ( m_media->fileType() == Media::Video )
    {
        if ( m_media->hasVideoTrack() == false )
            m_media->setFileType( Media::Audio );
    }
    else if ( m_media->fileType() == Media::Audio )
    {
        if ( m_media->hasVideoTrack() == true )
            m_media->setFileType( Media::Video );
    }
    if ( m_media->fileType() != Media::Audio )
    {
        // In theory the vout is created before the position actually changes.
        // If this happens to be true, we will have to re-add the old vout-waiting code
        Q_ASSERT_X( m_mediaPlayer->hasVout() == true, "metadata parsing",
                    "A Vout should have already been available. Please report the problem." );
        quint32     width, height;
        m_mediaPlayer->getSize( &width, &height );
        m_media->setWidth( width );
        m_media->setHeight( height );
        m_media->setFps( m_mediaPlayer->getFps() );
        if ( m_media->fps() == .0f )
        {
            qWarning() << "Invalid FPS for media:" << m_media->fileInfo()->absoluteFilePath();
            m_media->setFps( Clip::DefaultFPS );
        }
    }
    else
    {
        double fps = VLMC_PROJECT_GET_DOUBLE( "video/VLMCOutputFPS" );
        m_media->setFps( fps );
    }
    if ( m_media->fileType() == Media::Image )
        m_media->setLength( 10000 );
    else
        m_media->setLength( m_mediaPlayer->getLength() );

    m_media->setNbFrames( (m_media->lengthMS() / 1000) * m_media->fps() );

    m_media->emitMetaDataComputed();
#ifdef WITH_GUI
    //Setting time for snapshot :
    if ( m_media->fileType() != Media::Audio && m_media->hasSnapshot() == false )
    {
        computeSnapshot();
        return ;
    }
#endif
    emit computed();
}

#ifdef WITH_GUI
void
MetaDataWorker::computeSnapshot()
{
    QList<int>  cancel;
    cancel << libvlc_MediaPlayerEncounteredError << libvlc_MediaPlayerEndReached;

    m_mediaPlayer->setTime( m_mediaPlayer->getLength() / 3 );

    // Here we don't care about losing a TimeChanged event, so we don't lock before
    // the call to the method that would trigger the event. Anyway, TimeChanged event is triggered
    // almost often enough for us not to care if we missed one or not. However,
    // we don't want to catch one too early.
    m_mediaPlayer->configureWaitForEvent( libvlc_MediaPlayerTimeChanged, cancel, &checkEvent );
    LibVLCpp::MediaPlayer::EventWaitResult res = m_mediaPlayer->waitForEvent( 3000 );

    if ( res != LibVLCpp::MediaPlayer::Success )
    {
        qWarning() << "Got" << (res == LibVLCpp::MediaPlayer::Timeout ? "timeout" : "failure")
                      << "while launching metadata processing";
        emit failed( m_media );
        return ;
    }

    QTemporaryFile tmp;
    tmp.open();
    // the snapshot file will be removed when processed by the media.
    tmp.setAutoRemove( false );

    // Although this function is synchrone, we have to be in the main thread to
    // handle a QPixmap, hence the QueuedConnection
    connect( m_mediaPlayer, SIGNAL( snapshotTaken( const char* ) ),
             m_media, SLOT( snapshotReady( const char* ) ),
             Qt::QueuedConnection );
    m_mediaPlayer->takeSnapshot( tmp.fileName().toUtf8().constData(), 0, 0 );
    emit computed();
}
#endif

//void
//MetaDataWorker::prepareAudioSpectrumComputing()
//{
//    m_media->vlcMedia()->addOption( ":no-sout-video" );
//    m_media->vlcMedia()->addOption( ":sout=#transcode{}:smem" );
//    m_media->vlcMedia()->setAudioDataCtx( this );
//    m_media->vlcMedia()->setAudioLockCallback( reinterpret_cast<void*>( lock ) );
//    m_media->vlcMedia()->setAudioUnlockCallback( reinterpret_cast<void*>( unlock ) );
//    m_media->vlcMedia()->addOption( ":sout-transcode-acodec=fl32" );
//    m_media->vlcMedia()->addOption( ":no-sout-smem-time-sync" );
//    m_media->vlcMedia()->addOption( ":no-sout-keep" );
//    connect( m_mediaPlayer, SIGNAL( endReached() ), this, SLOT( generateAudioSpectrum() ), Qt::QueuedConnection );
//}
//
//void
//MetaDataWorker::lock( MetaDataWorker* metaDataWorker, uint8_t** pcm_buffer , unsigned int size )
//{
//    if ( metaDataWorker->m_audioBuffer == NULL )
//        metaDataWorker->m_audioBuffer = new unsigned char[size];
//    *pcm_buffer = metaDataWorker->m_audioBuffer;
//}
//
//void
//MetaDataWorker::unlock( MetaDataWorker* metaDataWorker, uint8_t* pcm_buffer,
//                                      unsigned int channels, unsigned int rate,
//                                      unsigned int nb_samples, unsigned int bits_per_sample,
//                                      unsigned int size, int pts )
//{
//    Q_UNUSED( rate );
//    Q_UNUSED( size );
//    Q_UNUSED( pts );
//
//    int bytePerChannelPerSample = bits_per_sample / 8;
//
//    int leftAverage = 0;
//    int rightAverage = 0;
//
//    int it = 0;
//    for ( unsigned int i = 0; i < nb_samples; i++)
//    {
//        int left = 0;
//        int right = 0;
//        for ( int u = 0 ; u < bytePerChannelPerSample; u++, it++ )
//        {
//            int increment = 0;
//            if ( channels == 2 )
//                increment = bytePerChannelPerSample;
//            left <<= 8;
//            left += pcm_buffer[ it ];
//            right <<= 8;
//            right += pcm_buffer[ it + increment ];
//        }
//        leftAverage += left;
//        rightAverage += right;
//    }
//    leftAverage /= nb_samples;
//    metaDataWorker->addAudioValue( leftAverage );
//}
//
//void
//MetaDataWorker::generateAudioSpectrum()
//{
//    disconnect( m_mediaPlayer, SIGNAL( endReached() ), this, SLOT( generateAudioSpectrum() ) );
//    m_mediaPlayer->stop();
////    AudioSpectrumHelper* audioSpectrum = new AudioSpectrumHelper( m_media->getAudioValues() );
////    audioSpectrum->setAutoDelete( true );
////    QThreadPool::globalInstance()->start( audioSpectrum );
//    m_media->emitAudioSpectrumComuted();
//    delete this;
//}
//
//void
//MetaDataWorker::addAudioValue( int value )
//{
//    m_media->audioValues()->append( value );
//}
//
