/*****************************************************************************
 * TrackControls.cpp: Widget used to configure a track
 *****************************************************************************
 * Copyright (C) 2008-2016 VideoLAN
 *
 * Authors: Ludovic Fauvet <etix@l0cal.com>
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

#include "TrackControls.h"

#include "timeline/GraphicsTrack.h"
#include "effectsengine/EffectStack.h"

#include "Backend/ITrack.h"

#include "Workflow/TrackWorkflow.h"

#include "ui_TrackControls.h"

#include <QInputDialog>
#include <QSettings>

TrackControls::TrackControls( GraphicsTrack* track, QWidget *parent ) :
    QWidget( parent ),
    m_ui( new Ui::TrackControls ),
    m_track( track )
{
    //FIXME: This type of things should be in the project settings
    QSettings   s( QSettings::UserScope, qApp->organizationName(), "timeline" );
    if ( track->mediaType() == Workflow::VideoTrack &&
         s.contains( "video" + QString::number( track->trackNumber() ) ) )
        m_title = s.value( "video" + QString::number( track->trackNumber() ) ).toString();
    else if ( track->mediaType() == Workflow::AudioTrack &&
                s.contains( "audio" + QString::number( track->trackNumber() ) ) )
        m_title = s.value( "audio" + QString::number( track->trackNumber() ) ).toString();

    m_ui->setupUi( this );
    if ( track->mediaType() != Workflow::VideoTrack )
        m_ui->fxButton->hide();
    setTrackDisabled( !m_track->isEnabled() );
    connect( m_ui->disableButton, SIGNAL( clicked(bool) ),
             this, SLOT( setTrackDisabled(bool) ) );
    connect( m_ui->trackLabel, SIGNAL( doubleClicked() ),
             this, SLOT( trackNameDoubleClicked() ) );
    connect( m_ui->fxButton, SIGNAL( clicked() ), this, SLOT( fxButtonClicked() ) );
    updateTextLabels();
}

TrackControls::~TrackControls()
{
    delete m_ui;
}

void
TrackControls::updateTextLabels()
{
    if ( m_track->mediaType() == Workflow::VideoTrack )
    {
        if ( m_title.isEmpty() == true )
            m_ui->trackLabel->setText( tr( "Video #%1" ).arg( QString::number( m_track->trackNumber() + 1 ) ) );
        else
            m_ui->trackLabel->setText( m_title );
    }
    else if ( m_track->mediaType() == Workflow::AudioTrack )
    {
        if ( m_title.isEmpty() )
            m_ui->trackLabel->setText( tr( "Audio #%1" ).arg( QString::number( m_track->trackNumber() + 1 ) ) );
        else
            m_ui->trackLabel->setText( m_title );
    }
}

void
TrackControls::changeEvent( QEvent *e )
{
    QWidget::changeEvent( e );
    switch ( e->type() ) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi( this );
        updateTextLabels();
        break;
    default:
        break;
    }
}

void
TrackControls::setTrackDisabled( bool disable )
{
    m_track->setTrackEnabled( !disable );
    if ( !disable )
    {
        if ( m_track->mediaType() == Workflow::VideoTrack )
            m_ui->disableButton->setIcon( QIcon( ":/images/trackon" ) );
        else if ( m_track->mediaType() == Workflow::AudioTrack )
            m_ui->disableButton->setIcon( QIcon( ":/images/hpon" ) );
    }
    else
    {
        if ( m_track->mediaType() == Workflow::VideoTrack )
            m_ui->disableButton->setIcon( QIcon( ":/images/trackoff" ) );
        else if ( m_track->mediaType() == Workflow::AudioTrack )
            m_ui->disableButton->setIcon( QIcon( ":/images/hpoff" ) );
    }
}

void
TrackControls::trackNameDoubleClicked()
{
    QString     name = QInputDialog::getText( nullptr, tr( "Rename track" ),
                                              tr( "Enter the track new name") );
    if ( name.isEmpty() == false )
    {
        QSettings   s( QSettings::UserScope, qApp->organizationName(), "timeline" );
        m_title = name;
        if ( m_track->mediaType() == Workflow::VideoTrack )
            s.setValue("video" + QString::number( m_track->trackNumber() ), name );
        else
            s.setValue("audio" + QString::number( m_track->trackNumber() ), name );
        updateTextLabels();
    }
}

void
TrackControls::fxButtonClicked()
{
    EffectStack *stack = new EffectStack( m_track->trackWorkflow()->input(), this );
    stack->show();
}
