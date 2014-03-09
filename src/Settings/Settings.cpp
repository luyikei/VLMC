/*****************************************************************************
 * Settings.cpp: Backend settings manager
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

#include "Settings.h"
#include "SettingValue.h"
#include "Tools/VlmcDebug.h"

#include <QByteArray>
#include <QFile>
#include <QWriteLocker>
#include <QReadLocker>
#include <QStringList>
#include <QXmlStreamWriter>

#include <QDomElement>


Settings::Settings(const QString &settingsFile)
    : m_settingsFile( NULL )
{
    if ( settingsFile.isEmpty() == false )
        m_settingsFile = new QFile( settingsFile );
}

Settings::~Settings()
{
    delete m_settingsFile;
}

bool
Settings::watchValue( const QString &key, QObject* receiver, const char *method, Qt::ConnectionType cType )
{
    SettingValue* s = value( key );
    if ( s != NULL )
    {
        QObject::connect( s, SIGNAL( changed( const QVariant& ) ),
                 receiver, method , cType );
        return true;
    }
    Q_ASSERT_X( false, __FILE__, "watching value without a created variable" );
    return false;
}

void
Settings::save( QXmlStreamWriter& project ) const
{
    QReadLocker lock( &m_rwLock );

    SettingMap::const_iterator     it = m_settings.begin();
    SettingMap::const_iterator     end = m_settings.end();

    project.writeStartElement( "settings" );
    for ( ; it != end; ++it )
    {
        if ( ( (*it)->flags() & SettingValue::Runtime ) != 0 )
            continue ;
        project.writeStartElement( "setting" );
        project.writeAttribute( "key", (*it)->key() );
        project.writeAttribute( "value", (*it)->get().toString() );
        project.writeEndElement();
    }
    project.writeEndElement();
}

bool
Settings::load( const QDomElement &root )
{
    QDomElement     element = root.firstChildElement( "settings" );
    if ( element.isNull() == true )
    {
        vlmcWarning() << "Invalid settings node";
        return false ;
    }
    QDomElement s = element.firstChildElement( "setting" );
    while ( s.isNull() == false )
    {
        QString     key = s.attribute( "key" );
        QString     value = s.attribute( "value" );

        if ( key.isEmpty() == true || value.isEmpty() == true )
            vlmcWarning() << "Invalid setting node.";
        else
            if ( setValue( key, value ) == false )
                vlmcWarning() << "Loaded invalid project setting:" << key;
        s = s.nextSiblingElement();
    }
    return true;
}

void
Settings::save() const
{
    if ( m_settingsFile == NULL )
        return ;
    QByteArray          settingsContent;
    QXmlStreamWriter    streamWriter( &settingsContent );
    save( streamWriter );
    m_settingsFile->open( QFile::WriteOnly );
    m_settingsFile->write( settingsContent );
    m_settingsFile->close();
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
    return false;
}

SettingValue*
Settings::value(const QString &key)
{
    QReadLocker lock( &m_rwLock );

    SettingMap::iterator it = m_settings.find( key );
    if ( it != m_settings.end() )
        return *it;
    return NULL;
}

SettingValue*
Settings::createVar(SettingValue::Type type, const QString &key, const QVariant &defaultValue, const char *name, const char *desc, SettingValue::Flags flags)
{
    QWriteLocker lock( &m_rwLock );

    if ( m_settings.contains( key ) )
        return NULL;
    SettingValue* val = new SettingValue( key, type, defaultValue, name, desc, flags );
    m_settings.insert( key, val );
    return val;
}

Settings::SettingList
Settings::group(const QString &groupName) const
{
    QReadLocker lock( &m_rwLock );
    SettingMap::const_iterator          it = m_settings.begin();
    SettingMap::const_iterator          ed = m_settings.end();
    SettingList        ret;

    QString grp = groupName + '/';
    for ( ; it != ed; ++it )
    {
        if ( (*it)->key().startsWith( grp ) )
            ret.push_back( *it );
    }
    return ret;
}
