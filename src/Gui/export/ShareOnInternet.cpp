/******************************************************************************
 * ShareOnInternet.cpp: Configure Video Export to Internet
 ******************************************************************************
 * Copyright (C) 2008-2016 VideoLAN
 *
 * Authors: Rohit Yadav <rohityadav89@gmail.com>
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

#include "Settings/Settings.h"
#include "ShareOnInternet.h"
#include "Tools/VlmcDebug.h"
#include "Services/AbstractSharingService.h"
#include "Services/YouTube/YouTubeService.h"

#include <QDesktopServices>
#include <QClipboard>
#include <QMessageBox>

ShareOnInternet::ShareOnInternet( QWidget* parent )
    : QDialog( parent, Qt::Sheet | Qt::Dialog )
{
    m_service = nullptr;
    m_serviceProvider = 0;
    m_ui.setupUi( this );
    m_ui.progressBar->setVisible( false );

    /* Get DevKey from VLMC settings */
    m_devKey = VLMC_GET_STRING( "youtube/DeveloperKey" );

    switch( m_serviceProvider )
    {
        case YOUTUBE:
            m_ui.username->setText( VLMC_GET_STRING( "youtube/Username" ) );
            m_ui.password->setText( VLMC_GET_STRING( "youtube/Password" ) );
            m_ui.title->setText( Core::instance()->project()->name() );
            break;
    }
}

ShareOnInternet::~ShareOnInternet()
{
    delete m_service;
}

void
ShareOnInternet::accept()
{
    if( m_ui.title->text().isEmpty() |
        m_ui.username->text().isEmpty() |
        m_ui.password->text().isEmpty() )
    {
        QMessageBox::critical( this, tr("Error"),
                               tr("'Username' or 'Password' or 'Title' cannot be empty. "
                                  "Please check these fields.") );
        return;
    }

    publish();
}

void
ShareOnInternet::publish()
{
    m_ui.publishButton->setEnabled( false );

    /* Check for service provider, if changes delete old one */
    if( m_serviceProvider != m_ui.serviceBox->currentIndex() )
    {
        if( m_service )
            delete m_service;

        m_service = nullptr;

        m_serviceProvider = m_ui.serviceBox->currentIndex();
    }

    if( !m_service )
    {
        switch( m_serviceProvider )
        {
            case VIMEO:  /* TODO: Implement services for Vimeo */
                vlmcDebug() << "[SHARE ON INTERNET]: VIMEO"; break;
            case YOUTUBE:
            default:
                m_service = new YouTubeService( m_devKey, getUsername(), getPassword() );
                vlmcDebug() << "[SHARE ON INTERNET]: YOUTUBE"; break;
        }
    }
    else
        m_service->setCredentials( getUsername(), getPassword() );

    m_ui.statusLabel->setText( tr("Authenticating...") );
    m_service->authenticate();

    connect( m_service, SIGNAL(authOver()), this, SLOT(authFinished()) );
    connect( m_service, SIGNAL(error(QString)), this, SLOT(serviceError(QString)) );
}

void
ShareOnInternet::authFinished()
{
    vlmcDebug() << "[SHARE ON INTERNET]: AUTH FINISHED";

    /*On Finish, extract out the auth token and upload a test video */
    disconnect( m_service, SIGNAL(authOver()), this, SLOT(authFinished()) );

    AbstractVideoData videoData = getVideoData();

    m_service->setVideoParameters( m_fileName, videoData );

    connect( m_service, SIGNAL(uploadOver(QString)), this, SLOT(uploadFinished(QString)));
    connect( m_service, SIGNAL(uploadProgress(qint64,qint64)),
             this, SLOT(uploadProgress(qint64,qint64)) );

    if( !m_service->upload() )
    {
        m_ui.publishButton->setEnabled( true );

        disconnect( m_service, SIGNAL(uploadOver(QString)), this, SLOT(uploadFinished(QString)));
        disconnect( m_service, SIGNAL(uploadProgress(qint64,qint64)),
                    this, SLOT(uploadProgress(qint64,qint64)) );
        disconnect( m_service, SIGNAL(error(QString)), this, SLOT(serviceError(QString)) );

        vlmcDebug() << "[SHARE ON INTERNET]: AUTH Failed";

        return;
    }
    m_ui.statusLabel->setText( tr("Authenticated!") );
    m_ui.progressBar->setEnabled( true );
    m_ui.progressBar->setVisible( true );
    m_ui.mainPanel->setEnabled( false );

    vlmcDebug() << "[SHARE ON INTERNET]: UPLOAD STARTED";
}

void
ShareOnInternet::uploadFinished( QString result )
{
    vlmcDebug() << "[SHARE ON INTERNET]: UPLOAD FINISHED";

    /* Add code here to abort stuff */
    m_ui.progressBar->setEnabled( false );
    m_ui.progressBar->setVisible( false );

    QString msg;
    if( result != "" )
    {
        msg = tr( "Your video has been uploaded."
                  "\nURL (copied to your clipboard):\n%1"
                  "\n\nOpen this in your default web browser?" ).arg( result );
        QApplication::clipboard()->setText( result );
    }
    else
        msg = tr( "Some error has occured while processing your video."
                  "\nPlease check with your video service provider." );

    if( QMessageBox::information( nullptr, tr("Video Uploaded"), msg,
            QMessageBox::Open | QMessageBox::Close ) == QMessageBox::Open )
    {
        QDesktopServices::openUrl( result );
    }

    /* Finish exec() */
    QDialog::accept();
}

void
ShareOnInternet::uploadProgress(qint64 received, qint64 total)
{
    if( total != 0 )
    {
        qint64 progress = received * 100 / total;
        m_ui.progressBar->setValue( progress );
        m_ui.statusLabel->setText( tr("%1 kB Uploaded").arg( received / 1024 ) );
    }
}

void
ShareOnInternet::serviceError(QString e)
{
    m_ui.statusLabel->setText( e );
    emit error( e );
}

QString
ShareOnInternet::getUsername() const
{
    return m_ui.username->text();
}

QString
ShareOnInternet::getPassword() const
{
    return m_ui.password->text();
}

AbstractVideoData
ShareOnInternet::getVideoData() const
{
    AbstractVideoData data;

    data.title       = m_ui.title->text();
    data.category    = m_ui.category->currentText().split(" & ").at( 0 );
    data.description = m_ui.description->toPlainText();
    data.keywords    = m_ui.keywords->text();
    data.isPrivate   = m_ui.videoPrivacy->isChecked();

    return data;
}

void
ShareOnInternet::setVideoFile( QString& fileName )
{
    m_fileName = fileName;
}
