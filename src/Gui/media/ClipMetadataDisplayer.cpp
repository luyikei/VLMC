/*****************************************************************************
 * ClipMetadataDisplayer.cpp: Display the basic metadata about a clip.
 *****************************************************************************
 * Copyright (C) 2008-2010 VideoLAN
 *
 * Authors: Hugo Beauzée-Luyssen <beauze.h@gmail.com>
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

#include "ClipMetadataDisplayer.h"

#include "Clip.h"
#include "Media.h"

#include <QTime>

ClipMetadataDisplayer::ClipMetadataDisplayer( const Clip *clip, QWidget *parent /*= NULL*/ ) :
    QWidget( parent ),
    m_ui( new Ui::ClipMetadataDisplayer ),
    m_watchedClip( clip )
{
    m_ui->setupUi( this );
    if ( clip != NULL )
        setWatchedClip( clip );
}

void
ClipMetadataDisplayer::metadataUpdated( const Media *media )
{
    QTime   duration;
    duration = duration.addSecs( m_watchedClip->lengthSecond() );

    updateInterface();
    //Duration
    m_ui->durationValueLabel->setText( duration.toString( "hh:mm:ss" ) );
    //Filename || title
    m_ui->nameValueLabel->setText( media->fileInfo()->fileName() );
    //Resolution
    m_ui->resolutionValueLabel->setText( QString::number( media->width() )
                                       + " x " + QString::number( media->height() ) );
    //FPS
    m_ui->fpsValueLabel->setText( QString::number( media->fps() ) );
    //nb tracks :
    m_ui->nbVideoTracksValueLabel->setText( QString::number( media->nbVideoTracks() ) );
    m_ui->nbAudioTracksValueLabel->setText( QString::number( media->nbAudioTracks() ) );
}

void
ClipMetadataDisplayer::setWatchedClip( const Clip *clip )
{
    if ( m_watchedMedia )
        disconnect( m_watchedMedia );

    m_watchedClip = clip;
    m_watchedMedia = clip->getMedia();
    if ( m_watchedMedia->isMetadataComputed() == true )
        metadataUpdated( m_watchedMedia );
    else
    {
        connect( m_watchedMedia, SIGNAL( metaDataComputed(const Media*) ),
                 this, SLOT( metadataUpdated( const Media*) ) );
    }
}

void
ClipMetadataDisplayer::updateInterface()
{
    m_ui->fpsLabel->setVisible( m_watchedMedia->hasVideoTrack() );
    m_ui->fpsValueLabel->setVisible( m_watchedMedia->hasVideoTrack() );
    m_ui->resolutionLabel->setVisible( m_watchedMedia->hasVideoTrack() );
    m_ui->resolutionValueLabel->setVisible( m_watchedMedia->hasVideoTrack() );
}
