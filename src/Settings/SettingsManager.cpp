/*****************************************************************************
 * SettingsManager.cpp: Backend settings manager
 *****************************************************************************
 * Copyright (C) 2008-2009 VideoLAN
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
#include "SettingValue.h"
#include "VlmcDebug.h"

#include <QSettings>
#include <QWriteLocker>
#include <QReadLocker>
#include <QStringList>

#include <QDomElement>

bool
SettingsManager::setValue( const QString &key,
                           const QVariant &value,
                           SettingsManager::Type type )
{
    if ( type == Project )
    {
        if ( m_xmlSettings.setValue( key, value ) )
            return true;
    }
    else// ( type == Vlmc && m_classicSettings.contains( key) == true )
    {
        SettingValue* v = m_classicSettings.value( key );
        if ( v != NULL )
        {
            v->set( value );
            if ( v->flags().testFlag( SettingValue::Runtime ) )
                return true;

            QSettings    sett;
            sett.setValue( key, value );
            sett.sync();
            return true;
        }
    }
    vlmcWarning() << "Setting" << key << "does not exist.";
    Q_ASSERT_X( false, __FILE__, "set value without a created variable" );
    return false;
}

SettingValue*
SettingsManager::value( const QString &key,
                        SettingsManager::Type type )
{
    SettingValue* result = NULL;
    if ( type == Project )
        result = m_xmlSettings.value( key );
    else
        result = m_classicSettings.value( key );
    Q_ASSERT_X( result != NULL, __FILE__, "get value without a created variable" );
    return result;
}

SettingsManager::SettingList
SettingsManager::group( const QString &groupName, SettingsManager::Type type )
{
    if ( type == Project )
        return m_xmlSettings.group( groupName );
    else if ( type == Vlmc )
        return m_classicSettings.group( groupName );
    Q_ASSERT_X( false, __FILE__, "Unknown setting type" );
    return SettingList();
}

bool
SettingsManager::watchValue( const QString &key,
                             QObject* receiver,
                             const char *method,
                             SettingsManager::Type type,
                             Qt::ConnectionType cType )
{
    SettingValue* s = value( key, type );
    if ( s != NULL )
    {
        connect( s, SIGNAL( changed( const QVariant& ) ),
                 receiver, method , cType );
        return true;
    }
    Q_ASSERT_X( false, __FILE__, "watching value without a created variable" );
    return false;
}

void
SettingsManager::save() const
{
    m_classicSettings.lockForRead();

    QSettings       sett;
    SettingsContainer::SettingMap::const_iterator it = m_classicSettings.settings().begin();
    SettingsContainer::SettingMap::const_iterator ed = m_classicSettings.settings().end();

    for ( ; it != ed; ++it )
    {
        if ( ( (*it)->flags() & SettingValue::Private ) != 0 )
            continue ;
        sett.setValue( (*it)->key(), (*it)->get() );
    }
    sett.sync();
}

void
SettingsManager::save( QXmlStreamWriter& project ) const
{
    m_xmlSettings.lockForRead();

    SettingsContainer::SettingMap::const_iterator     it = m_xmlSettings.settings().begin();
    SettingsContainer::SettingMap::const_iterator     end = m_xmlSettings.settings().end();

    project.writeStartElement( "project" );
    for ( ; it != end; ++it )
    {
        if ( ( (*it)->flags() & SettingValue::Private ) != 0 )
            continue ;
        project.writeStartElement( "property" );
        project.writeAttribute( "key", (*it)->key() );
        project.writeAttribute( "value", (*it)->get().toString() );
        project.writeEndElement();
    }
    project.writeEndElement();
}

bool
SettingsManager::load( const QDomElement &root )
{
    //For now it only handle a project node.
    QDomElement     element = root.firstChildElement( "project" );
    if ( element.isNull() == true )
    {
        vlmcWarning() << "Invalid settings node";
        return false ;
    }
    QDomElement s = element.firstChildElement();
    while ( s.isNull() == false )
    {
        QString     key = s.attribute( "key" );
        QString     value = s.attribute( "value" );

        if ( key.isEmpty() == true || value.isEmpty() == true )
            vlmcWarning() << "Invalid setting node.";
        else
            if ( setValue( key, value, SettingsManager::Project ) == false )
                vlmcWarning() << "Loaded invalid project setting:" << key;
        s = s.nextSiblingElement();
    }
    return true;
}

SettingValue*
SettingsManager::createVar( SettingValue::Type type, const QString &key,
                            const QVariant &defaultValue, const char *name,
                            const char *desc, SettingsManager::Type varType /*= Vlmc*/,
                            SettingValue::Flags flags /*= SettingValue::Nothing*/ )
{
    SettingValue    *val = NULL;
    if ( varType == Vlmc )
        val = m_classicSettings.createVar( type, key, defaultValue, name, desc, flags );
    else if ( varType == Project )
        val = m_xmlSettings.createVar( type, key, defaultValue, name, desc, flags );
    Q_ASSERT_X( val != NULL, __FILE__, "creating an already created variable" );
    return val;
}


bool
SettingsManager::SettingsContainer::setValue(const QString &key, const QVariant &value)
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
SettingsManager::SettingsContainer::value(const QString &key)
{
    QReadLocker lock( &m_rwLock );

    SettingMap::iterator it = m_settings.find( key );
    if ( it != m_settings.end() )
        return *it;
    return NULL;
}

SettingValue*
SettingsManager::SettingsContainer::createVar(SettingValue::Type type, const QString &key, const QVariant &defaultValue, const char *name, const char *desc, SettingValue::Flags flags)
{
    QWriteLocker lock( &m_rwLock );

    if ( m_settings.contains( key ) )
        return NULL;
    SettingValue* val = new SettingValue( key, type, defaultValue, name, desc, flags );
    m_settings.insert( key, val );
    return val;
}

const SettingsManager::SettingsContainer::SettingMap&
SettingsManager::SettingsContainer::settings() const
{
    return m_settings;
}

SettingsManager::SettingList
SettingsManager::SettingsContainer::group(const QString &groupName) const
{
    QReadLocker lock( &m_rwLock );
    SettingMap::const_iterator          it = m_settings.begin();
    SettingMap::const_iterator          ed = m_settings.end();
    SettingsManager::SettingList        ret;

    QString grp = groupName + '/';
    for ( ; it != ed; ++it )
    {
        if ( (*it)->key().startsWith( grp ) )
            ret.push_back( *it );
    }
    return ret;
}

void
SettingsManager::SettingsContainer::lockForRead() const
{
    m_rwLock.lockForRead();
}

void
SettingsManager::SettingsContainer::unlock() const
{
    m_rwLock.unlock();
}
