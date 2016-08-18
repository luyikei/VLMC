/*****************************************************************************
 * ClipMetadataDisplayer.cpp: Display the basic metadata about a clip.
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "ClipMetadataDisplayer.h"

#include "Media/Clip.h"
#include "Media/Media.h"

#include <QTime>

ClipMetadataDisplayer::ClipMetadataDisplayer( QWidget *parent /*= nullptr*/ ) :
    QWidget( parent ),
    m_ui( new Ui::ClipMetadataDisplayer ),
    m_watchedClip( nullptr )
{
    m_ui->setupUi( this );
}

ClipMetadataDisplayer::~ClipMetadataDisplayer()
{
    delete m_ui;
}

void
ClipMetadataDisplayer::metadataUpdated()
{
    QTime   duration;
    duration = duration.addSecs( m_watchedClip->lengthSecond() );

    const auto* input = m_watchedMedia->input();
    updateInterface();
    //Duration
    m_ui->durationValueLabel->setText( duration.toString( "hh:mm:ss" ) );
    //Filename || title
    m_ui->nameValueLabel->setText( m_watchedMedia->title() );
    //Resolution
    m_ui->resolutionValueLabel->setText( QString::number( input->width() )
                                       + " x " + QString::number( input->height() ) );
    //FPS
    m_ui->fpsValueLabel->setText( QString::number( input->fps() ) );
    //nb tracks :
    m_ui->nbVideoTracksValueLabel->setText( QString::number( input->nbVideoTracks() ) );
    m_ui->nbAudioTracksValueLabel->setText( QString::number( input->nbAudioTracks() ) );
    //Path:
    m_ui->pathValueLabel->setText( m_watchedMedia->mrl() );
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
    m_watchedMedia = clip->media();
    connect( m_watchedClip, SIGNAL( unloaded( Clip* ) ), this, SLOT( clipDestroyed( Clip* ) ) );
    metadataUpdated();
}

void
ClipMetadataDisplayer::updateInterface()
{
    bool visible = m_watchedMedia->input()->hasVideo();
    m_ui->fpsLabel->setVisible( visible );
    m_ui->fpsValueLabel->setVisible( visible );
    m_ui->resolutionLabel->setVisible( visible );
    m_ui->resolutionValueLabel->setVisible( visible );
}
