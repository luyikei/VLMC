/*****************************************************************************
 * SettingsManager.cpp: Backend settings manager
 *****************************************************************************
 * Copyright (C) 2008-2009 VideoLAN
 *
 * Authors: Clement CHAVANCE <kinder@vlmc.org>
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

#include <QSettings>
#include <QWriteLocker>
#include <QReadLocker>
#include <QStringList>

#include <QtDebug>
#include <QDomElement>

void
SettingsManager::setValue( const QString &key,
                           const QVariant &value,
                           SettingsManager::Type type )
{
    if ( type == Project && m_xmlSettings.contains( key ) == true )
        m_xmlSettings[key]->set( value );
    else if ( type == Vlmc && m_classicSettings.contains( key) == true )
        m_classicSettings[key]->set( value );
    else
    {
        Q_ASSERT_X( false, __FILE__, "set value without a created variable" );
        qWarning() << "Setting" << key << "does not exist.";
    }
    return ;
}

void
SettingsManager::setImmediateValue( const QString &key,
                                    const QVariant &value,
                                    SettingsManager::Type type )
{
    QWriteLocker    wlock( &m_rwLock );
    SettingHash  *settMap;
    if ( type == Project )
        settMap = &m_xmlSettings;
    else if ( type == Vlmc )
    {
        QSettings    sett;
        sett.setValue( key, value );
        sett.sync();
        settMap = &m_classicSettings;
    }
    if ( settMap->contains( key ) )
    {
        settMap->value( key )->set( value );
    }
    else
    {
        Q_ASSERT_X( false, __FILE__, "set immediate value without a created variable" );
        qWarning() << "Setting" << key << "does not exist.";
    }
    return ;
}

SettingValue*
SettingsManager::value( const QString &key,
                        SettingsManager::Type type )
{
    QReadLocker rl( &m_rwLock );

    if ( type == Project )
    {
        if ( m_xmlSettings.contains( key ) )
            return m_xmlSettings.value( key );
    }
    else
    {
        if ( m_classicSettings.contains( key ) )
            return m_classicSettings.value( key );
    }
    Q_ASSERT_X( false, __FILE__, "get value without a created variable" );
    qWarning() << "Setting" << key << "does not exist.";
    return NULL;
}

SettingsManager::SettingHash
SettingsManager::group( const QString &groupName, SettingsManager::Type type )
{
    SettingsManager::SettingHash        ret;
    QReadLocker                         rl( &m_rwLock );

    QString grp = groupName + '/';
    if ( type == Project )
    {
         SettingHash::const_iterator it = m_xmlSettings.begin();
         SettingHash::const_iterator ed = m_xmlSettings.end();

         for ( ; it != ed; ++it )
         {
             if ( it.key().startsWith( grp ) )
                 ret.insert( it.key(), it.value() );
         }
    }
    else if ( type == Vlmc )
    {
         SettingHash::const_iterator it = m_classicSettings.begin();
         SettingHash::const_iterator ed = m_classicSettings.end();

         for ( ; it != ed; ++it )
         {
             if ( it.key().startsWith( grp ) )
             {
                 ret.insert( it.key(), it.value() );
             }
         }
    }
    return ret;
}

bool
SettingsManager::watchValue( const QString &key,
                             QObject* receiver,
                             const char *method,
                             SettingsManager::Type type,
                             Qt::ConnectionType cType )
{
    QReadLocker rl( &m_rwLock );

    if ( type == Project && m_xmlSettings.contains( key ) )
    {
        connect( m_xmlSettings[key], SIGNAL( changed( const QVariant& ) ),
                 receiver, method );
        return true;
    }
    else if ( type == Vlmc )
    {
        if ( m_classicSettings.contains( key ) )
        {
            connect( m_classicSettings[key], SIGNAL( changed( const QVariant& ) ),
                    receiver, method, cType );
            return true;
        }
    }
    Q_ASSERT_X( false, __FILE__, "watching value without a created variable" );
    return false;
}

void
SettingsManager::save() const
{
    QReadLocker rl( &m_rwLock );
    QSettings    sett;
    SettingHash::const_iterator it = m_classicSettings.begin();
    SettingHash::const_iterator ed = m_classicSettings.end();

    for ( ; it != ed; ++it )
        sett.setValue( it.key(), it.value()->get() );
    sett.sync();
}

void
SettingsManager::save( QXmlStreamWriter& project ) const
{
    SettingHash::const_iterator     it = m_xmlSettings.begin();
    SettingHash::const_iterator     end = m_xmlSettings.end();

    project.writeStartElement( "project" );
    while ( it != end )
    {
        project.writeStartElement( "property" );
        project.writeAttribute( "key", it.key() );
        project.writeAttribute( "value", it.value()->get().toString() );
        ++it;
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
        qWarning() << "Invalid settings node";
        return false ;
    }
    QWriteLocker    wLock( &m_rwLock );
    QDomElement s = element.firstChild().toElement();
    while ( s.isNull() == false )
    {
        QString     key = s.attribute( "key" );
        QString     value = s.attribute( "value" );

        if ( key.isEmpty() == true || value.isEmpty() == true )
            qWarning() << "Invalid setting node.";
        else
            setValue( key, value, SettingsManager::Project );
        s = s.nextSibling().toElement();
    }
    return true;
}

void
SettingsManager::createVar( SettingValue::Type type, const QString &key,
                            const QVariant &defaultValue, const char *name,
                            const char *desc, SettingsManager::Type varType /*= Vlmc*/ )
{
    QWriteLocker    wlock( &m_rwLock );

    if ( varType == Vlmc && m_classicSettings.contains( key ) == false )
        m_classicSettings.insert( key, new SettingValue( type, defaultValue, name, desc ) );
    else if ( varType == Project && m_xmlSettings.contains( key ) == false )
        m_xmlSettings.insert( key, new SettingValue( type, defaultValue, name, desc ) );
    else
        Q_ASSERT_X( false, __FILE__, "creating an already created variable" );
}

void
SettingsManager::flush()
{
    QWriteLocker    wl( &m_rwLock );
    m_tmpXmlSettings.clear();
    m_tmpClassicSettings.clear();
}

SettingsManager::SettingsManager()
    : m_classicSettings(),
    m_xmlSettings(),
    m_tmpClassicSettings(),
    m_tmpXmlSettings(),
    m_rwLock()
{
}

SettingsManager::~SettingsManager()
{
}
