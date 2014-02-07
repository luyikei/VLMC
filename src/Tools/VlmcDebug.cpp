/*****************************************************************************
 * VlmcDebug.cpp: Debug tools for VLMC
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

#include <QCoreApplication>
#include <QStringList>

#include "SettingsManager.h"
#include "VlmcDebug.h"

VlmcDebug::VlmcDebug() : m_logFile( NULL )
{
    //setup log level :
    VLMC_CREATE_PRIVATE_PREFERENCE_INT( "private/LogLevel", 0 );
    QStringList args = qApp->arguments();
    if ( args.indexOf( QRegExp( "-vv+" ) ) >= 0 )
        m_currentLogLevel = VlmcDebug::Debug;
    else if ( args.contains( "-v" ) == true )
        m_currentLogLevel = VlmcDebug::Verbose;
    else
        m_currentLogLevel = VlmcDebug::Quiet;
    SettingsManager::getInstance()->setValue( "private/LogLevel", m_currentLogLevel, SettingsManager::Vlmc );

//    int pos = args.indexOf( QRegExp( "--logfile=.*" ) );
//    if ( pos > 0 )
//    {
//        QString arg = args[pos];
//        QString logFile = arg.mid( 10 );
//        if ( logFile.length() <= 0 )
//            qWarning() << tr("Invalid value supplied for argument --logfile" );
//        else
//            SettingsManager::getInstance()->setValue( "private/LogFile", logFile, SettingsManager::Vlmc );
//    }


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

VlmcDebug::~VlmcDebug()
{
    delete m_logFile;
}

void
VlmcDebug::setup()
{
    qInstallMsgHandler( VlmcDebug::vlmcMessageHandler );
}

void
VlmcDebug::logFileChanged( const QVariant& logFileV )
{
    QString logFile = logFileV.toString();
    if ( logFile.isEmpty() == true )
    {
        m_logFile->close();
        return ;
    }
    if ( logFile == m_logFile->fileName() )
    {
        fprintf( stderr, "Trying to use the same log file. Nothing will be done" );
        return ;
    }
    m_logFile->close();
    m_logFile->setFileName( logFile );
    m_logFile->open( QFile::Append | QFile::Truncate );
}

void
VlmcDebug::vlmcMessageHandler( QtMsgType type, const char* msg )
{
    //FIXME: This is ok as long as we guarantee no log messages will happen after we
    // uninstall the hook
    VlmcDebug* self = VlmcDebug::getInstance();
    if ( self->m_logFile != NULL )
    {
        self->m_logFile->write( msg );
        self->m_logFile->write( "\n" );
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
