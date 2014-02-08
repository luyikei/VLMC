/*****************************************************************************
 * Transcoder.cpp: Handle file transcoding.
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

#include "Transcoder.h"

#include "LibVLCpp/VLCMedia.h"
#include "LibVLCpp/VLCMediaPlayer.h"
#include "Media.h"
#include "MetaDataManager.h"
#include "NotificationZone.h"
#include "SettingsManager.h"

#include <QFileInfo>

Transcoder::Transcoder( Media* media ) :
        m_media( media )
{
    connect( this, SIGNAL( notify( QString ) ),
             NotificationZone::getInstance(), SLOT( notify( QString ) ) );
    connect( this, SIGNAL( progress( float ) ),
             NotificationZone::getInstance(), SLOT( progressUpdated( float ) ) );
}

void
Transcoder::transcodeToPs()
{
    QString             outputDir = VLMC_PROJECT_GET_STRING( "general/Workspace" );
    LibVLCpp::Media     *media = new LibVLCpp::Media( m_media->fileInfo()->absoluteFilePath() );

    if ( outputDir.length() == 0 )
        outputDir = m_media->fileInfo()->absolutePath();
    m_destinationFile = outputDir + '/' + m_media->fileInfo()->baseName() + ".ps";
    QString         option = ":sout=file://" + m_destinationFile;
    media->addOption( option.toUtf8().constData() );
    LibVLCpp::MediaPlayer   *mp = new LibVLCpp::MediaPlayer( "Transcoder", media );
    connect( mp, SIGNAL( positionChanged( float ) ), this, SIGNAL( progress( float ) ) );
    connect( mp, SIGNAL( endReached() ), this, SLOT( transcodeFinished() ) );
    emit notify( "Transcoding " + m_media->fileInfo()->absoluteFilePath() + " to " + m_destinationFile );
    mp->play();
}

void
Transcoder::transcodeFinished()
{
    m_media->setFilePath( m_destinationFile );
    MetaDataManager::getInstance()->computeMediaMetadata( m_media );
    emit done();
    emit notify( m_media->fileInfo()->fileName() + ": Transcode finished" );
}
