/******************************************************************************
 * YouTubeService.cpp: YouTube Service APIs
 ******************************************************************************
 * Copyright (C) 2010 VideoLAN
 *
 * Authors: Rohit Yadav <rohityadav89 AT gmail.com>
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

#include "YouTubeAuthenticator.h"
#include "YouTubeService.h"
#include "YouTubeUploader.h"

#include <QAuthenticator>
#include <QByteArray>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QSslError>

#include <QDebug>

using namespace YouTube;

YouTubeService::YouTubeService( const QString& devKey,
                                const QString& username,
                                const QString& password )
{
    /* Stores Credentials */
    m_devKey   = devKey;
    m_username = username;
    m_password = password;

    /* Pointers for Authenticator and Uploader Objects */
    m_auth     = NULL;
    m_uploader = NULL;
}

YouTubeService::~YouTubeService()
{
    /* Clean exit, checks and deletes objects */
    if( m_auth )
        delete m_auth;

    if( m_uploader )
        delete m_uploader;
}

void
YouTubeService::authenticate()
{
    /* Auth object: Checks, creates and sets credentials */
    if( !m_auth )
        m_auth = new YouTubeAuthenticator( this, m_username, m_password );
    else
        m_auth->setCredentials( m_username, m_password );

    /* Tell world on successful authentication */
    connect( m_auth, SIGNAL( authOver() ),
             this, SIGNAL( authOver() ) );

    /* On authentication error, m_auth sends the error token */
    connect( m_auth, SIGNAL( authError( QString ) ),
             this, SLOT( authError( QString ) ) );

    m_auth->authenticate();
}

bool
YouTubeService::upload()
{
    /* Checks authentication and then only uploads video file */
    if( m_auth->isAuthenticated() )
    {
        /* Uploader object: Checks, creates and sets video file */
        if( !m_uploader )
            m_uploader = new YouTubeUploader( this, m_fileName );
        else
            m_uploader->setVideoFile( m_fileName );

        m_uploader->setVideoData( m_videoData );

        /* Tell world on successful uploading */
        connect( m_uploader, SIGNAL( uploadOver( QString ) ),
                 this, SIGNAL( uploadOver( QString ) ) );

        /* Connects upload progress */
        connect( m_uploader, SIGNAL( uploadProgress( qint64, qint64 ) ),
                 this, SIGNAL( uploadProgress( qint64, qint64 ) ) );

        return m_uploader->upload();
    }
    return false;
}

const QString&
YouTubeService::getAuthString()
{
    return m_auth->getAuthString();
}

const QString&
YouTubeService::getDeveloperKey()
{
    return m_devKey;
}

const VideoData&
YouTubeService::getVideoData()
{
    return m_uploader->getVideoData();
}

void
YouTubeService::setCredentials( const QString& username, const QString& password )
{
    m_username = username;
    m_password = password;
}

void
YouTubeService::setDeveloperKey( const QString& devKey )
{
    m_devKey = devKey;
}

void
YouTubeService::setVideoParameters( const QString& fileName, const VideoData& data )
{
    m_fileName  = fileName;
    m_videoData = data;
}

void
YouTubeService::authError( QString e )
{
    qDebug() << "[YT SERVICE]: AUTH ERROR " << e;

    if( e == "BadAuthentication" )
        m_error = BadAuthentication;
    else
    if( e == "CaptchaRequired")
        m_error = CaptchaRequired;
    else
    if( e == "ServiceUnavailable")
        m_error = ServiceUnavailable;
    else
        m_error = UnknownError;

    emit error( e );
}

void
YouTubeService::networkError( QNetworkReply::NetworkError e )
{
    QString errString;

    /* Checks error code and emits error string */
    switch( e )
    {
        case QNetworkReply::ConnectionRefusedError:
        case QNetworkReply::RemoteHostClosedError:
        case QNetworkReply::HostNotFoundError:
        case QNetworkReply::TimeoutError:
            errString = "Network Connection Error";
            m_error = NetworkError; break;

        case QNetworkReply::OperationCanceledError:
            errString = "Operation Aborted";
            m_error = Abort; break;

        case QNetworkReply::SslHandshakeFailedError:
            errString = "SSL Error";
            m_error = SSLError; break;

        case QNetworkReply::ProxyConnectionRefusedError:
        case QNetworkReply::ProxyConnectionClosedError:
        case QNetworkReply::ProxyNotFoundError:
        case QNetworkReply::ProxyTimeoutError:
            errString = "Proxy Connection Error";
            m_error = ProxyError; break;

        case QNetworkReply::ProxyAuthenticationRequiredError:
            errString = "Proxy Authentication Error";
            m_error = ProxyAuthError; break;

        case QNetworkReply::ContentAccessDenied:
        case QNetworkReply::ContentOperationNotPermittedError:
        case QNetworkReply::ContentNotFoundError:
            errString = "Remote Content Error";
            m_error = ContentError; break;

        case QNetworkReply::ProtocolUnknownError:
        case QNetworkReply::UnknownNetworkError:
        case QNetworkReply::UnknownProxyError:
        case QNetworkReply::UnknownContentError:
        default:
            errString = "Unknown Error";
            m_error = UnknownError;
    }

    qDebug() << "[NETWORK ERROR]: " << e << ": " << errString;
    emit error( errString );

    /* Ignore Content and Abort errors */
    if( m_error == ContentError || m_error == Abort )
        return;

    QNetworkReply *reply = static_cast<QNetworkReply *>( sender() );

    if( m_state == UploadStart )
    {
        disconnect( reply, SIGNAL( finished() ),
                    m_uploader, SLOT( uploadFinished() ) );
        disconnect( reply, SIGNAL( uploadProgress( qint64, qint64 ) ),
                    m_uploader, SIGNAL( uploadProgress( qint64, qint64 ) ) );
    }

    reply->close();
    reply->deleteLater();
}

#ifndef QT_NO_OPENSSL
void
YouTubeService::sslErrors( QNetworkReply* reply, const QList<QSslError> &errors )
{
    m_error = SSLError;

    QString errorString;
    foreach (const QSslError &error, errors)
    {
        if (!errorString.isEmpty())
            errorString += ", ";
        errorString += error.errorString();
    }

    /* Prompt for insecure connection */
    if ( QMessageBox::warning(NULL, tr("YouTube Authentication"),
                             tr("Connection may be insecure, do you want to continue?"
                                " One or more SSL errors has occurred: %1").arg(errorString),
                             QMessageBox::Ignore | QMessageBox::Abort) == QMessageBox::Ignore )
    {
        reply->ignoreSslErrors();
    }
}
#endif
