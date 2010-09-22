/*****************************************************************************
 * YouTubeAuthenticator.cpp: Auth Class for YouTube
 *****************************************************************************
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

#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QString>
#include <QStringList>
#include <QUrl>

#include <QDebug>

using namespace YouTube;

YouTubeAuthenticator::YouTubeAuthenticator( YouTubeService* service, 
                                            const QString& username,
                                            const QString& password )
{
    /* Stores pointer to main service object and user credentials */
    m_service  = service;
    m_username = username;
    m_password = password;

    m_nam = new QNetworkAccessManager( this );

    /* If SSL is available, handle SSL errors for better security */
    #ifndef QT_NO_OPENSSL
    connect( m_nam, SIGNAL( sslErrors( QNetworkReply*, QList<QSslError> ) ),
             m_service, SLOT( sslErrors( QNetworkReply*, QList<QSslError> ) ) );
    #endif

    authInit();
    setPOSTData();
}

YouTubeAuthenticator::~YouTubeAuthenticator()
{
    delete m_nam;
}

void
YouTubeAuthenticator::authInit()
{
    m_isAuthenticated = false;
    m_nick.clear();
    m_authString.clear();
    m_authError.clear();
}

void
YouTubeAuthenticator::authenticate()
{
    /* Network request to be sent */
    QNetworkRequest request = getNetworkRequest();

    /* Sends the auth details using HTTPS POST*/
    QNetworkReply* reply = m_nam->post( request, getPOSTData() );
    m_service->m_state = AuthStart;

    connect( reply, SIGNAL( finished() ), this, SLOT( authFinished() ) );
    connect( reply, SIGNAL( error(QNetworkReply::NetworkError ) ),
             m_service, SLOT( networkError( QNetworkReply::NetworkError ) ) );
}

void
YouTubeAuthenticator::authFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply *>( sender() );
    QByteArray data = reply->readAll();

    if( setAuthData( data ) )
        m_service->m_state = AuthFinish;

    disconnect( reply, SIGNAL( finished() ), this, SLOT( authFinished() ) );
    disconnect( reply, SIGNAL( error( QNetworkReply::NetworkError ) ),
                m_service, SLOT( networkError( QNetworkReply::NetworkError ) ) );

    reply->close();
    reply->deleteLater();
}

void
YouTubeAuthenticator::error( QString& e )
{
    qDebug() << "[YouTube AUTH ERROR]: " << e;
    emit authError( e );
}

bool
YouTubeAuthenticator::isAuthenticated()
{
    return m_isAuthenticated;
}

const QByteArray&
YouTubeAuthenticator::getPOSTData()
{
    return m_postData;
}

QNetworkRequest
YouTubeAuthenticator::getNetworkRequest()
{
    authInit();
    QUrl url( LOGIN_URL );

    QNetworkRequest request;
    request.setHeader( QNetworkRequest::ContentTypeHeader,
                       "application/x-www-form-urlencoded" );
    request.setUrl( url );

    return request;
}

const QString&
YouTubeAuthenticator::getAuthString()
{
    return m_authString;
}


const QString&
YouTubeAuthenticator::getNick()
{
    return m_nick;
}

void
YouTubeAuthenticator::setCredentials( const QString& username, const QString& password )
{
    m_username = username;
    m_password = password;
    setPOSTData();
}

void
YouTubeAuthenticator::setPOSTData()
{
    m_postData.clear();
    m_postData += QString("accountType=HOSTED_OR_GOOGLE&Email=%1&Passwd=%2"
                          "&service=youtube&source=VLMC").arg(m_username, m_password);
}

void
YouTubeAuthenticator::setServiceProvider( YouTubeService *service )
{
    m_service = service;
}

bool
YouTubeAuthenticator::setAuthData( QByteArray& data )
{
    /* Parses data received after sending auth details */
    QStringList lines = QString( data ).split( "\n", QString::SkipEmptyParts );

    foreach( QString line, lines )
    {
        QStringList tokenList = line.split( "=" );

        if( tokenList.at(0) == "Error" )
        {
            m_authError = tokenList.at(1);
            error( m_authError );
            continue;
        }

        if( tokenList.at(0) == "Auth" )
        {
            m_authString = tokenList.at(1);
            continue;
        }

        if( tokenList.at(0) == "YouTubeUser" )
        {
            m_nick = tokenList.at(1);
        }
    }

    if( !m_authString.isEmpty() && !m_nick.isEmpty() && m_authError.isEmpty() )
        m_isAuthenticated = true;

    emit authOver();
    return m_isAuthenticated;
}
