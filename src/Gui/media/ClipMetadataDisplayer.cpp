/*****************************************************************************
 * ClipMetadataDisplayer.cpp: Display the basic metadata about a clip.
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

#include "ClipMetadataDisplayer.h"

#include "Clip.h"
#include "ISource.h"
#include "Media.h"

#include <QTime>

ClipMetadataDisplayer::ClipMetadataDisplayer( QWidget *parent /*= NULL*/ ) :
    QWidget( parent ),
    m_ui( new Ui::ClipMetadataDisplayer ),
    m_watchedClip( NULL )
{
    m_ui->setupUi( this );
}

void
ClipMetadataDisplayer::metadataUpdated( const Media *media )
{
    QTime   duration;
    duration = duration.addSecs( m_watchedClip->lengthSecond() );

    const Backend::ISource* source = media->source();
    updateInterface();
    //Duration
    m_ui->durationValueLabel->setText( duration.toString( "hh:mm:ss" ) );
    //Filename || title
    m_ui->nameValueLabel->setText( media->fileInfo()->fileName() );
    //Resolution
    m_ui->resolutionValueLabel->setText( QString::number( source->width() )
                                       + " x " + QString::number( source->height() ) );
    //FPS
    m_ui->fpsValueLabel->setText( QString::number( source->fps() ) );
    //nb tracks :
    m_ui->nbVideoTracksValueLabel->setText( QString::number( source->nbVideoTracks() ) );
    m_ui->nbAudioTracksValueLabel->setText( QString::number( source->nbAudioTracks() ) );
    //Path:
    m_ui->pathValueLabel->setText( media->fileInfo()->absoluteFilePath() );
    //Workspace:
    workspaceStateChanged( media->isInWorkspace() );
}

void
ClipMetadataDisplayer::clear()
{
    m_ui->durationValueLabel->setText( "---" );
    //Filename || title
    m_ui->nameValueLabel->setText( "---" );
    //Resolution
    m_ui->resolutionValueLabel->setText( "---" );
    //FPS
    m_ui->fpsValueLabel->setText( "---" );
    //nb tracks :
    m_ui->nbVideoTracksValueLabel->setText( "---" );
    m_ui->nbAudioTracksValueLabel->setText( "---" );
    //Path:
    m_ui->pathValueLabel->setText( "---" );
}

void
ClipMetadataDisplayer::clipDestroyed( Clip* clip )
{
    if ( m_watchedClip == clip )
        clear();
}

void
ClipMetadataDisplayer::setWatchedClip( const Clip *clip )
{
    if ( m_watchedMedia )
        disconnect( m_watchedMedia );
    if ( m_watchedClip )
        disconnect( m_watchedClip );

    m_watchedClip = clip;
    m_watchedMedia = clip->getMedia();
    connect( m_watchedClip, SIGNAL( unloaded( Clip* ) ), this, SLOT( clipDestroyed( Clip* ) ) );
    if ( m_watchedMedia->isMetadataComputed() == true )
        metadataUpdated( m_watchedMedia );
    else
    {
        connect( m_watchedMedia, SIGNAL( metaDataComputed(const Media*) ),
                 this, SLOT( metadataUpdated( const Media*) ) );
    }
    connect( m_watchedMedia, SIGNAL( workspaceStateChanged( bool ) ),
             this, SLOT( workspaceStateChanged( bool ) ) );
}

void
ClipMetadataDisplayer::workspaceStateChanged( bool state )
{
    if ( state == true )
        m_ui->inProjectWorkspaceValueLabel->setPixmap( QPixmap( ":/images/ok" ).scaled( 16, 16 ) );
    else
        m_ui->inProjectWorkspaceValueLabel->setPixmap( QPixmap( ":/images/ko" ).scaled( 16, 16 ) );
}

void
ClipMetadataDisplayer::updateInterface()
{
    bool visible = m_watchedMedia->source()->hasVideo();
    m_ui->fpsLabel->setVisible( visible );
    m_ui->fpsValueLabel->setVisible( visible );
    m_ui->resolutionLabel->setVisible( visible );
    m_ui->resolutionValueLabel->setVisible( visible );
}
