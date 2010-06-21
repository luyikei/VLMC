/*****************************************************************************
 * RendererSettings.cpp: Edit the output file parameters
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

#include "RendererSettings.h"
#include "SettingsManager.h"

#include "ui_RendererSettings.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QCompleter>
#include <QDirModel>
#include <QFileInfo>

RendererSettings::RendererSettings()
{
    m_ui.setupUi( this );
    setWindowFlags( windowFlags() | Qt::Sheet ); // Qt::Sheet is for UI on Mac
    connect( m_ui.outputFileNameButton, SIGNAL(clicked() ),
             this, SLOT(selectOutputFileName() ) );
    m_ui.width->setValue( VLMC_PROJECT_GET_INT( "video/VideoProjectWidth" ) );
    m_ui.height->setValue( VLMC_PROJECT_GET_INT( "video/VideoProjectHeight" ) );
    m_ui.fps->setValue( VLMC_PROJECT_GET_DOUBLE( "video/VLMCOutputFPS" ) );

    QCompleter* completer = new QCompleter( this );
    completer->setModel( new QDirModel( completer ) );
    m_ui.outputFileName->setCompleter( completer );
}

void
RendererSettings::selectOutputFileName()
{
    QString outputFileName =
            QFileDialog::getSaveFileName( NULL, tr ( "Enter the output file name" ),
                                          QDir::currentPath(), tr( "Videos(*.avi *.mpg)" ) );
    m_ui.outputFileName->setText( outputFileName );
}

void
RendererSettings::accept()
{
    if ( width() <= 0 || height() <= 0 || fps() <= .0f )
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
