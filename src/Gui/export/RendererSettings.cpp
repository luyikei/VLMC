/*****************************************************************************
 * RendererSettings.cpp: Edit the output file parameters
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

#include "Media/Media.h"
#include "RendererSettings.h"
#include "Settings/Settings.h"

#include <QCompleter>
#include <QDirModel>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QSslSocket>

RendererSettings::RendererSettings( bool shareOnInternet )
{
    m_ui.setupUi( this );
    auto project = Core::instance()->project();

    if( shareOnInternet )
    {
        m_ui.outputLabel->setVisible( false );
        m_ui.outputFileName->setVisible( false );
        m_ui.outputFileNameButton->setVisible( false );
        m_ui.outputFileName->setText(
            VLMC_GET_STRING( "vlmc/TempFolderLocation" ) + "/" +
            project->name() + "-vlmc.mp4" );
        setWindowTitle( tr("Export Settings: Publish on Internet") );
    }

    m_ui.width->setValue( project->width() );
    m_ui.height->setValue( project->height() );
    m_ui.fps->setValue( project->fps() );
    m_ui.videoQuality->setValue( project->videoBitrate() );
    m_ui.audioQuality->setValue( project->audioBitrate() );
    m_ui.nbChannels->setValue( project->nbChannels() );
    m_ui.sampleRate->setValue( project->sampleRate() );

    QCompleter* completer = new QCompleter( this );
    completer->setModel( new QDirModel( completer ) );
    m_ui.outputFileName->setCompleter( completer );

    connect( m_ui.outputFileNameButton, SIGNAL( clicked() ),
             this, SLOT(selectOutputFileName() ) );
    connect( m_ui.videoPresetBox, SIGNAL( activated( int ) ),
             this, SLOT( updateVideoPreset( int ) ) );

    if( !QSslSocket::supportsSsl() )
	    QMessageBox::information(0, "SSL Error",
		    "This build has no OpenSSL support, video upload may not work.");
}

void
RendererSettings::selectOutputFileName()
{
    QString outputFileName =
            QFileDialog::getSaveFileName( nullptr, tr ( "Enter the output file name" ),
                                          QDir::currentPath(), tr( "Videos(%1)" ).arg( Media::VideoExtensions ) );
    m_ui.outputFileName->setText( outputFileName );
}

void
RendererSettings::setPreset( quint32 width, quint32 height, double fps )
{
    m_ui.width->setValue( width );
    m_ui.height->setValue( height );
    m_ui.fps->setValue( fps );
}

void
RendererSettings::updateVideoPreset( int index )
{
    m_ui.width->setEnabled( false );
    m_ui.height->setEnabled( false );
    m_ui.fps->setEnabled( false );

    switch( index )
    {
        case QVGA:  setPreset( 320, 240, 30);   break;
        case VGA:   setPreset( 640, 480, 30);   break;
        case SVGA:  setPreset( 800, 600, 30);   break;
        case XVGA:  setPreset( 1024, 768, 30);  break;
        case P480:  setPreset( 720, 480, 29.97);   break;
        case P576:  setPreset( 720, 576, 25);   break;
        case P720:  setPreset( 1280, 720, 29.97);  break;
        case P1080: setPreset( 1920, 1080, 29.97); break;
        case Custom:
        default:
             m_ui.width->setEnabled( true );
             m_ui.height->setEnabled( true );
             m_ui.fps->setEnabled( true );
    }
}

void
RendererSettings::accept()
{
    if ( width() <= 0 || height() <= 0 || fps() <= .0f || audioBitrate() <= 0 || videoBitrate() <= 0 )
    {
        QMessageBox::warning( this, tr( "Invalid parameters" ),
                              tr( "Please enter valid rendering parameters" ) );
        return;
    }

    QFileInfo fileInfo( m_ui.outputFileName->text() );

    if ( outputFileName().isEmpty() || fileInfo.isDir() || !fileInfo.dir().exists() )
    {
        QMessageBox::warning( this, tr( "Invalid parameters" ),
                              tr( "Please provide a valid output file!" ) );
        m_ui.outputFileName->setFocus();
        return;
    }

    if ( fileInfo.isFile() )
    {
        QMessageBox::StandardButton b =
                QMessageBox::question( this, tr( "File already exists!" ),
                                       tr( "Output file already exists, do you want to "
                                           "overwrite it?" ),
                                       QMessageBox::Yes | QMessageBox::No,
                                       QMessageBox::No );
        if ( b == QMessageBox::No )
            return;
    }

    QDialog::accept();
}

quint32
RendererSettings::width() const
{
    return m_ui.width->value();
}

quint32
RendererSettings::height() const
{
    return m_ui.height->value();
}

double
RendererSettings::fps() const
{
    return m_ui.fps->value();
}

QString
RendererSettings::aspectRatio() const
{
    //FIXME:
    return "16/10";
}

quint32
RendererSettings::nbChannels() const
{
    return m_ui.nbChannels->value();
}

quint32
RendererSettings::sampleRate() const
{
    return m_ui.sampleRate->value();
}

quint32
RendererSettings::videoBitrate() const
{
    return m_ui.videoQuality->value();
}

quint32
RendererSettings::audioBitrate() const
{
    return m_ui.audioQuality->value();
}

QString
RendererSettings::outputFileName() const
{
    return m_ui.outputFileName->text();
}
