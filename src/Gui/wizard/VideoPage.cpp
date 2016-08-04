/*****************************************************************************
 * VideoPage.cpp: Wizard page for configuring video settings
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "VideoPage.h"
#include "Settings/Settings.h"

VideoPage::VideoPage( QWidget* parent ) :
    QWizardPage( parent )
{
    ui.setupUi( this );

    setTitle( tr( "New project wizard" ) );
    setSubTitle( tr( "Configure Video settings" ) );

    setFinalPage( true );

    connect( ui.comboBoxVideoPresets, SIGNAL( currentIndexChanged(int) ),
             this, SLOT( updateVideoPresets() ) );
    connect( ui.comboBoxAudioPresets, SIGNAL( currentIndexChanged(int) ),
             this, SLOT( updateAudioPresets() ) );
    registerField( "fps", ui.doubleSpinBoxVideoFPS, "value", SIGNAL( valueChanged( double ) ) );
    registerField( "width", ui.spinBoxVideoWidth );
    registerField( "height", ui.spinBoxVideoHeight );
    registerField( "aspectratio", ui.lineEditVideoAspectRatio );
    registerField( "samplerate", ui.comboBoxAudioSamplerate, "currentText" );
    registerField( "channels", ui.spinBoxAudioChannels );
    registerField( "abitrate", ui.comboBoxAudioBitrate, "currentText" );
    registerField( "vbitrate", ui.spinBoxVideoBitrate );
}

void
VideoPage::changeEvent( QEvent* e )
{
    QWizardPage::changeEvent( e );
    switch ( e->type() )
    {
    case QEvent::LanguageChange:
        ui.retranslateUi( this );
        break;
    default:
        break;
    }
}

int
VideoPage::nextId() const
{
    return -1;
}

void
VideoPage::initializePage()
{
    int     projectFps = 30;
    int     projectHeight = 300;
    int     projectWidth = 480;
    int     sampleRate = 44000;

    ui.comboBoxVideoPresets->setCurrentIndex( 0 );
    ui.comboBoxAudioPresets->setCurrentIndex( 1 );
    ui.spinBoxVideoWidth->setValue( projectWidth );
    ui.spinBoxVideoHeight->setValue( projectHeight );
    ui.doubleSpinBoxVideoFPS->setValue( projectFps );
    ui.comboBoxAudioBitrate->setCurrentIndex( ui.comboBoxAudioBitrate->findText( "256 Kbps" ) );

    switch ( sampleRate )
    {
    case 48000:
        ui.comboBoxAudioSamplerate->setCurrentIndex( HZ_48000 );
        break;
    case 22000:
        ui.comboBoxAudioSamplerate->setCurrentIndex( HZ_22050 );
        break;
    case 11000:
        ui.comboBoxAudioSamplerate->setCurrentIndex( HZ_11025 );
        break;
    case 44000:
    default:
        ui.comboBoxAudioSamplerate->setCurrentIndex( HZ_44100 );
        break;
    }
}

bool
VideoPage::validatePage()
{
    return true;
}

void
VideoPage::cleanupPage()
{
}

void
VideoPage::setVideoFormEnabled( bool enabled )
{
    ui.spinBoxVideoWidth->setEnabled( enabled );
    ui.spinBoxVideoHeight->setEnabled( enabled );
}

void
VideoPage::setAudioFormEnabled( bool enabled )
{
    ui.spinBoxAudioChannels->setEnabled( enabled );
    ui.comboBoxAudioSamplerate->setEnabled( enabled );
}

void
VideoPage::updateVideoPresets()
{
    if ( ui.comboBoxVideoPresets->currentIndex() == 0 )
        setVideoFormEnabled( true );
    else
        setVideoFormEnabled( false );

    switch ( ui.comboBoxVideoPresets->currentIndex() )
    {
    case PRESET_VideoCustom: break;
    case PRESET_480i:
        setVideoResolution( 720, 480 );
        setVideoFPS( 30.0 );
        break;
    case PRESET_576i:
        setVideoResolution( 720, 576 );
        setVideoFPS( 25.0 );
        break;
    case PRESET_480p:
        setVideoResolution( 720, 480 );
        setVideoFPS( 29.97 );
        break;
    case PRESET_576p:
        setVideoResolution( 720, 576 );
        setVideoFPS( 29.97 );
        break;
    case PRESET_720p:
        setVideoResolution( 1280, 720 );
        setVideoFPS( 29.97 );
        break;
    case PRESET_1080i:
        setVideoResolution( 1920, 1080 );
        setVideoFPS( 30.0 );
        break;
    case PRESET_1080p:
        setVideoResolution( 1920, 1080 );
        setVideoFPS( 29.97 );
        break;
    }
}

void
VideoPage::updateAudioPresets()
{
    if ( ui.comboBoxAudioPresets->currentIndex() == 0 )
        setAudioFormEnabled( true );
    else
        setAudioFormEnabled( false );

    switch ( ui.comboBoxAudioPresets->currentIndex() )
    {
    case PRESET_AudioCustom: break;
    case PRESET_STEREO:
        ui.spinBoxAudioChannels->setValue( 2 );
        break;
    case PRESET_MONO:
        ui.spinBoxAudioChannels->setValue( 1 );
        break;
    }
}

void
VideoPage::setVideoResolution( int width, int height )
{
    ui.spinBoxVideoWidth->setValue( width );
    ui.spinBoxVideoHeight->setValue( height );
}

void
VideoPage::setVideoFPS( double fps )
{
    ui.doubleSpinBoxVideoFPS->setValue( fps );
}
