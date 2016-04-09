/*****************************************************************************
 * Settings.cpp: Backend settings manager
 *****************************************************************************
 * Copyright (C) 2008-2016 VideoLAN
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

#include "Settings.h"
#include "SettingValue.h"
#include "Tools/VlmcDebug.h"

#include <QByteArray>
#include <QFile>
#include <QWriteLocker>
#include <QReadLocker>
#include <QStringList>
#include <QXmlStreamWriter>
#include <QFileInfo>
#include <QDir>

#include <QJsonDocument>
#include <QJsonObject>


Settings::Settings(const QString &settingsFile)
    : m_settingsFile( nullptr )
{
    setSettingsFile( settingsFile );
}

Settings::~Settings()
{
    qDeleteAll( m_settings );
    delete m_settingsFile;
}

void
Settings::setSettingsFile(const QString &settingsFile)
{
    delete m_settingsFile;
    if ( settingsFile.isEmpty() == false )
    {
        QFileInfo fInfo( settingsFile );
        if ( fInfo.exists() == false )
        {
            QDir dir = fInfo.dir();
            if ( dir.exists() == false )
                dir.mkpath( fInfo.absolutePath() );
        }
        m_settingsFile = new QFile( settingsFile );
    }
    else
        m_settingsFile = nullptr;
}

bool
Settings::load()
{
    if ( m_settingsFile->open( QFile::ReadOnly ) == false )
    {
        vlmcWarning() << "Failed to open settings file" << m_settingsFile->fileName();
        return false;
    }
    QJsonParseError error;
    m_jsonObject = QJsonDocument::fromJson( m_settingsFile->readAll(), &error ).object();
    if ( error.error != QJsonParseError::NoError )
    {
        vlmcWarning() << "Failed to load settings file" << m_settingsFile->fileName();
        vlmcWarning() << error.errorString();
        return false;
    }

    for ( auto it = m_jsonObject.constBegin();
          it != m_jsonObject.constEnd();
          ++it
          )
    {
        if ( (*it).type() == QJsonValue::Object )
            continue ;
        if ( setValue( it.key(), (*it).toVariant() ) == false )
            vlmcWarning() << "Loaded invalid project setting:" << it.key();

    }

    m_settingsFile->close();
    return true;
}

bool
Settings::save()
{
    if ( m_settingsFile == nullptr )
        return false;
    QJsonDocument doc;

    QReadLocker lock( &m_rwLock );

    QJsonObject top;
    for ( const auto& val : m_settings )
    {
        if ( ( val->flags() & SettingValue::Runtime ) != 0 )
            continue ;
        if ( top.insert( val->key(), QJsonValue::fromVariant( val->get() ) ) == top.end() )
            vlmcWarning() << "Failed to set:" << val->key();
    }
    doc.setObject( top );

    m_settingsFile->open( QFile::WriteOnly );
    m_settingsFile->write( doc.toJson( QJsonDocument::Compact ) );
    m_settingsFile->close();
    return true;
}

bool
Settings::setValue(const QString &key, const QVariant &value)
{
    SettingMap::iterator   it = m_settings.find( key );
    if ( it != m_settings.end() )
    {
        (*it)->set( value );
        return true;
    }
    Q_ASSERT_X( false, __FILE__, "setting value without a created variable" );
    return false;
}
void
Settings::restoreDefaultValues()
{
    QReadLocker lock( &m_rwLock );
    for (auto s : m_settings)
    {
        s->restoreDefault();
    }
}
SettingValue*
Settings::value(const QString &key)
{
    QReadLocker lock( &m_rwLock );

    SettingMap::iterator it = m_settings.find( key );
    if ( it != m_settings.end() )
        return *it;
    Q_ASSERT_X( false, __FILE__, "fetching value without a created variable" );
    return nullptr;
}

SettingValue*
Settings::createVar(SettingValue::Type type, const QString &key, const QVariant &defaultValue, const char *name, const char *desc, SettingValue::Flags flags)
{
    QWriteLocker lock( &m_rwLock );

    if ( m_settings.contains( key ) )
        return nullptr;
    SettingValue* val = new SettingValue( key, type, defaultValue, name, desc, flags );
    m_settings.insert( key, val );
    return val;
}

Settings::SettingList
Settings::group(const QString &groupName) const
{
    QReadLocker lock( &m_rwLock );
    SettingList        ret;

    QString grp = groupName + '/';
    for ( auto s : m_settings )
    {
        if ( s->key().startsWith( grp ) )
            ret.push_back( s );
    }
    return ret;
}
