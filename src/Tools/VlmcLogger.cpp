/*****************************************************************************
 * VlmcLogger.cpp: Debug tools for VLMC
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

#include "SettingsManager.h"
#include "VlmcLogger.h"
#include "VlmcDebug.h"

#include <QCoreApplication>
#include <QDesktopServices>
#include <QStringList>
#include <QThread>

VlmcLogger::VlmcLogger() : m_logFile( NULL )
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
    SettingsManager* settingsManager = SettingsManager::getInstance();
    settingsManager->setValue( "private/LogLevel", m_currentLogLevel, SettingsManager::Vlmc );
    settingsManager->watchValue( "private/LogLevel", this, SLOT(logLevelChanged( const QVariant& )),
                                 SettingsManager::Vlmc, Qt::DirectConnection );

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


//    QVariant setVal = SettingsManager::getInstance()->value( "private/LogFile", "log.vlmc", SettingsManager::Vlmc );
//    SettingsManager::getInstance()->watchValue( "private/LogFile", this,
//                                              SLOT( logFileChanged( const QVariant& ) ),
//                                              SettingsManager::Vlmc );
//    QObject::connect( qApp,
//                      SIGNAL( aboutToQuit() ),
//                      this,
//                      SLOT( deleteLater() ) );
//    QString logFile = setVal.toString();
//    if ( logFile.isEmpty() == false )
//    {
//        m_logFile = new QFile( logFile );
//        m_logFile->open( QFile::WriteOnly | QFile::Truncate );
//    }
}

VlmcLogger::~VlmcLogger()
{
    if ( m_logFile )
        fclose( m_logFile );
}

void
VlmcLogger::setup()
{
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

    VlmcLogger* self = VlmcLogger::getInstance();
    if ( self->m_logFile != NULL )
    {
        //FIXME: Messages are not guaranteed to arrive in order
        self->writeToFile(msg);
    }
    if ( (int)type < (int)self->m_currentLogLevel )
        return ;
    switch ( type )
    {
    case QtDebugMsg:
        fprintf(stderr, "%s\n", msg);
        break;
    case QtWarningMsg:
        fprintf(stderr, "%s\n", msg);
        break;
    case QtCriticalMsg:
        fprintf(stderr, "%s\n", msg);
        break;
    case QtFatalMsg:
        fprintf(stderr, "%s\n", msg);
        abort();
    }
}
