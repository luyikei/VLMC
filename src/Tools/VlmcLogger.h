/*****************************************************************************
 * VlmcLogger.h: Debug tools for VLMC
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

#ifndef VLMCDEBUG_H
#define VLMCDEBUG_H

#include <QObject>
#include <QDebug>

#include <cstdio>

#include "Singleton.hpp"

/**
 *  \warning    Do not use qDebug() qWarning() etc... from here, unless you know exactly what you're doing
 *              Chances are very high that you end up with a stack overflow !!
 */
class   VlmcLogger : public QObject, public Singleton<VlmcLogger>
{
    Q_OBJECT

    public:
        // Keep the same order as Qt's message levels, since that's what we're
        // going to use for comparing log levels in the message handler
        enum    LogLevel
        {
            Debug = QtDebugMsg,
            Verbose = QtWarningMsg,
            // This means both qCritical() & qFatal() will be displayed in quiet mode
            Quiet = QtCriticalMsg
        };

        static void     vlmcMessageHandler( QtMsgType type, const char* msg );
        void            setup();
    private:
        VlmcLogger();
        virtual ~VlmcLogger();
        void            writeToFile(const char* msg);

        FILE*           m_logFile;
        LogLevel        m_currentLogLevel;

    private slots:
        void            logLevelChanged( const QVariant& logLevel );

        friend class    Singleton<VlmcLogger>;
};

#endif // VLMCDEBUG_H
