/*****************************************************************************
 * VlmcLogger.cpp: Debug tools for VLMC
 *****************************************************************************
 * Copyright (C) 2008-2014 VideoLAN
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

#include <QCoreApplication>
#include <QStringList>
#include <QThread>

#include "Main/Core.h"
#include "Settings/SettingsManager.h"
#include "Tools/VlmcLogger.h"
#include "Tools/VlmcDebug.h"


VlmcLogger::VlmcLogger()
    : m_logFile( NULL )
    , m_backendLogLevel( Backend::IBackend::None )
{
}

VlmcLogger::~VlmcLogger()
{
    if ( m_logFile )
        fclose( m_logFile );
}

void
VlmcLogger::setup()
{
    //setup log level :
    {
        SettingValue* logLevel = VLMC_CREATE_PREFERENCE( SettingValue::Int, "private/LogLevel", (int)VlmcLogger::Quiet,
                                                        "", "", SettingValue::Private | SettingValue::Clamped | SettingValue::Runtime );
        logLevel->setLimits((int)Debug, (int)Verbose);
        // Purposedly destroying the setting value, as we need to use the manager for other operations.
        //FIXME: Actually I'm not sure for setting the value since this is a private variable.
    }
    QStringList args = qApp->arguments();
    if ( args.indexOf( QRegExp( "-vv+" ) ) >= 0 )
        m_currentLogLevel = VlmcLogger::Debug;
    else if ( args.contains( "-v" ) == true )
        m_currentLogLevel = VlmcLogger::Verbose;
    else
        m_currentLogLevel = VlmcLogger::Quiet;
    Settings* settings = Core::getInstance()->settings();
    settings->setValue( "private/LogLevel", m_currentLogLevel );
    settings->watchValue( "private/LogLevel", this, SLOT(logLevelChanged( const QVariant& )), Qt::DirectConnection );

    int pos = args.indexOf( QRegExp( "--logfile=.*" ) );
    if ( pos > 0 )
    {
        QString arg = args[pos];
        QString logFile = arg.mid( 10 );
        if ( logFile.length() <= 0 )
            vlmcWarning() << tr("Invalid value supplied for argument --logfile" );
        else
        {
            m_logFile = fopen(logFile.toLocal8Bit().data(), "w");
        }
    }

    pos = args.indexOf( QRegExp( "--vlcverbose=.*" ) );
    if ( pos > 0 )
    {
        QString arg = args[pos];
        QString vlcLogLevelStr = arg.mid( 13 );

        if ( vlcLogLevelStr.length() <= 0 )
            vlmcWarning() << tr("Invalid value supplied for argument --vlcverbose" );
        else
        {
            bool ok = false;
            unsigned int vlcLogLevel = vlcLogLevelStr.toUInt( &ok );
            if ( ok == true )
            {
                if ( vlcLogLevel >= 3 )
                    m_backendLogLevel = Backend::IBackend::Debug;
                else if ( vlcLogLevel == 2 )
                    m_backendLogLevel = Backend::IBackend::Warning;
                else if ( vlcLogLevel == 1 )
                    m_backendLogLevel = Backend::IBackend::Error;
            }
            else
                vlmcWarning() << tr("Invalid value supplied for argument --vlcverbose" );
        }
    }
    Backend::IBackend* backend = Backend::getBackend();
    backend->setLogHandler( this, &VlmcLogger::backendLogHandler );

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    qInstallMessageHandler( VlmcLogger::vlmcMessageHandler );
#else
    qInstallMsgHandler( VlmcLogger::vlmcMessageHandler );
#endif
}

void
VlmcLogger::logLevelChanged( const QVariant &logLevel )
{

    Q_ASSERT_X(logLevel.toInt() >= (int)VlmcLogger::Debug &&
               logLevel.toInt() <= (int)VlmcLogger::Quiet,
               "Setting log level", "Invalid value for log level");
    m_currentLogLevel = (VlmcLogger::LogLevel)logLevel.toInt();
}

/*********************************************************************
* Don't use anything which might use qDebug/qWarning/... below here. *
*********************************************************************/

void
VlmcLogger::writeToFile(const char *msg)
{
    flockfile( m_logFile );
    fputs( msg, m_logFile );
    fputc( '\n', m_logFile );
    funlockfile( m_logFile );
}

void
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
VlmcLogger::vlmcMessageHandler( QtMsgType type, const QMessageLogContext&, const QString& str )
{
    const QByteArray byteArray = str.toLocal8Bit();
    const char* msg = byteArray.constData();
#else
VlmcLogger::vlmcMessageHandler( QtMsgType type, const char* msg )
{
#endif
    //FIXME: This is ok as long as we guarantee no log message will arrive after
    // we uninstall the hook

    VlmcLogger* self = Core::getInstance()->logger();
    if ( self->m_logFile != NULL )
    {
        //FIXME: Messages are not guaranteed to arrive in order
        self->writeToFile(msg);
    }
    self->outputToConsole( (int)type, msg );
}

void
VlmcLogger::outputToConsole( int level, const char *msg )
{
    if ( level < (int)m_currentLogLevel )
        return ;
    switch ( (QtMsgType)level )
    {
    case QtDebugMsg:
        fprintf(stdout, "%s\n", msg);
        break;
    case QtWarningMsg:
        fprintf(stdout, "%s\n", msg);
        break;
    case QtCriticalMsg:
        fprintf(stdout, "%s\n", msg);
        break;
    case QtFatalMsg:
        fprintf(stderr, "%s\n", msg);
        abort();
    }
}

void
VlmcLogger::backendLogHandler(void *data, Backend::IBackend::LogLevel logLevel, const char* msg )
{
    VlmcLogger* self = reinterpret_cast<VlmcLogger*>( data );
    char* newMsg = NULL;
    if ( asprintf( &newMsg, "[%s] T #%p [Backend] %s", qPrintable( QTime::currentTime().toString( "hh:mm:ss.zzz" ) ),
              QThread::currentThreadId(), msg ) < 0 )
        return ;

    if ( self->m_logFile != NULL )
        self->writeToFile( newMsg );
    if ( logLevel < self->m_backendLogLevel )
    {
        free( newMsg );
        return ;
    }
    switch ( logLevel )
    {
        case Backend::IBackend::Debug:
            self->outputToConsole( Debug, newMsg );
            break;
        case Backend::IBackend::Warning:
            self->outputToConsole( Verbose, newMsg );
            break;
        case Backend::IBackend::Error:
            self->outputToConsole( Quiet, newMsg );
            break;
        default:
            Q_ASSERT(false);
    }
    free( newMsg );
}
