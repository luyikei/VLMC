/*****************************************************************************
 * YouTubeAuthenticator.h: Auth Class for YouTube
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

#ifndef YOUTUBEAUTHENTICATOR_H
#define YOUTUBEAUTHENTICATOR_H

#include <QObject>

#define LOGIN_URL "https://www.google.com/youtube/accounts/ClientLogin"

class QByteArray;
class QNetworkAccessManager;
class QNetworkRequest;
class YouTubeService;

class YouTubeAuthenticator : public QObject
{
    Q_OBJECT

    public:
        YouTubeAuthenticator( YouTubeService* service = 0, 
                              const QString& username = "",
                              const QString& password = "" );
        ~YouTubeAuthenticator();

        void  authenticate();
        bool  isAuthenticated();

        const QString& getAuthString();
        const QString& getNick();

        void  setServiceProvider( YouTubeService* service );
        void  setCredentials( const QString& username, const QString& password );
        bool  setAuthData( QByteArray& data );

    private:
        void                   authInit();
        void                   error( QString& error );

        QNetworkRequest        getNetworkRequest();
        const QByteArray&      getPOSTData();

        void                   setPOSTData();

        /* Youtube Credentials */
        QString                m_username;
        QString                m_password;

        /* HTTP/S POST HEADER */
        QByteArray             m_postData;

        /* Youtube tokens */
        QString                m_authString;
        QString                m_nick;
        bool                   m_isAuthenticated;
        QString                m_authError;

        YouTubeService*        m_service;
        QNetworkAccessManager* m_nam;

    private slots:
        void                   authFinished();

    signals:
        void                   authOver();
        void                   authError( QString );
};
#endif // YOUTUBEAUTHENTICATOR_H
