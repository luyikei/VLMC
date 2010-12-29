/*****************************************************************************
 * EffectsEngine.cpp: Manage the effects plugins.
 *****************************************************************************
 * Copyright (C) 2008-2010 VideoLAN
 *
 * Authors: Hugo Beauzée-Luyssen <beauze.h@gmail.com>
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

#include "EffectsEngine.h"

#include "Effect.h"
#include "EffectInstance.h"
#include "Types.h"

#include <QDesktopServices>
#include <QDir>
#include <QProcess>
#include <QSettings>
#include <QXmlStreamWriter>

#include <QtDebug>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

EffectsEngine::EffectsEngine()
{
    m_cache = new QSettings( QDesktopServices::storageLocation(
                    QDesktopServices::CacheLocation ) + "/effects",
                             QSettings::IniFormat, this );
    //Create the names entry. A bit ugly but faster (I guess...) afterward.
    m_names.push_back( QStringList() );
    m_names.push_back( QStringList() );
    m_names.push_back( QStringList() );
    m_names.push_back( QStringList() );
}

EffectsEngine::~EffectsEngine()
{
}

Effect*
EffectsEngine::effect( const QString& name )
{
    QHash<QString, Effect*>::iterator   it = m_effects.find( name );
    if ( it != m_effects.end() )
        return it.value();
    return NULL;
}

bool
EffectsEngine::loadEffect( const QString &fileName )
{
    Effect*         e = new Effect( fileName );
    QString         name;
    Effect::Type    type;

    if ( m_cache->contains( fileName + "/name" ) == true &&
         m_cache->contains( fileName + "/type" ) == true )
    {
        name = m_cache->value( fileName + "/name" ).toString();
        int     typeInt = m_cache->value( fileName + "/type" ).toInt();
        if ( typeInt < Effect::Unknown || typeInt > Effect::Mixer3 )
            qWarning() << "Invalid plugin type.";
        else
        {
            type = static_cast<Effect::Type>( typeInt );
            if ( m_effects.contains( name ) == true )
            {
                delete e;
                return false;
            }
            m_effects[name] = e;
            m_names[type].push_back( name );
            emit effectAdded( e, name, type );
            return true;
        }
    }
    if ( e->load() == false || m_effects.contains( e->name() ) == true )
    {
        delete e;
        return false;
    }
    m_effects[e->name()] = e;
    m_cache->setValue( fileName + "/name", e->name() );
    m_cache->setValue( fileName + "/type", e->type() );
    name = e->name();
    type = e->type();
    m_names[type].push_back( name );
    emit effectAdded( e, name, type );
    return true;
}

void
EffectsEngine::browseDirectory( const QString &path )
{
    QDir    dir( path );
    const QFileInfoList& files = dir.entryInfoList( QDir::Files | QDir::NoDotAndDotDot |
                                              QDir::Readable ); //| QDir::Executable );
    foreach ( const QFileInfo& file, files )
    {
        if ( file.isDir() )
            browseDirectory( file.absoluteFilePath() );
        else
            loadEffect( file.absoluteFilePath() );
    }
}

void
EffectsEngine::loadEffects()
{
    QStringList     pathList;

#if defined( Q_OS_UNIX )
    const QProcessEnvironment &env = QProcessEnvironment::systemEnvironment();
    if ( env.contains( "FREI0R_PATH" ) == true )
        pathList = env.value( "FREI0R_PATH" ).split( ':' );
    else
    {
        //Refer to http://www.piksel.org/frei0r/1.2/spec/group__pluglocations.html
        pathList << QString( QDir::homePath() + "/.frei0r-1/lib/" ) <<
                    QString("/usr/local/lib/frei0r-1/") <<
                    QString("/usr/lib/frei0r-1/" );
    }
#elif defined ( Q_OS_WIN32 )
    TCHAR       appDir[128];
    if ( GetModuleFileName( NULL, appDir, 128 ) > 0 )
    {
        TCHAR     *pos = strrchr( appDir, '\\' );
        if ( pos == NULL )
        {
            qWarning() << "Can't use ModuleFileName:" << appDir;
            return ;
        }
        *pos = 0;
        pathList << QString( appDir ) + "/effects/";
    }
    else
    {
        qWarning() << "Failed to get application directory. Using current path.";
        pathList << QDir::currentPath() + "/effects/";
    }
#endif
    qDebug() << "Loading effects from:" << pathList;
    foreach ( const QString &path, pathList )
    {
        if ( QFile::exists( path ) == true )
        {
            qDebug() << "\tScanning" << path << "for effects";
            browseDirectory( path );
        }
    }
}

const QStringList&
EffectsEngine::effects(Effect::Type type) const
{
    return m_names[type];
}
