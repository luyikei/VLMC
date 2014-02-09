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

static SettingsManager::SettingList::iterator
getPair( SettingsManager::SettingList &list, const QString &key )
{
    SettingsManager::SettingList::iterator        it = list.begin();
    SettingsManager::SettingList::iterator        end = list.end();

    while ( it != end )
    {
        if ( *it == key )
            return it;
        ++it;
    }
    return end;
}

static bool
contains( const SettingsManager::SettingList &list, const QString &key )
{
    SettingsManager::SettingList::const_iterator        it = list.constBegin();
    SettingsManager::SettingList::const_iterator        end = list.constEnd();

    while ( it != end )
    {
        if ( *it == key )
            return true;
        ++it;
    }
    return false;
}

void
SettingsManager::setValue( const QString &key,
                           const QVariant &value,
                           SettingsManager::Type type )
{
    if ( type == Project )
    {
        SettingList::iterator   it = getPair( m_xmlSettings, key );
        if ( it != m_xmlSettings.end() )
        {
            (*it).value->set( value );
            return ;
        }
    }
    else// ( type == Vlmc && m_classicSettings.contains( key) == true )
    {
        SettingList::iterator   it = getPair( m_classicSettings, key );
        if ( it != m_classicSettings.end() )
        {
            SettingValue* v = (*it).value;

            // We don't want private values in our QSettings, that would be
            // saved in the preference files, and they're called private for a reason
            v->set( value );
            if ( v->flags().testFlag( SettingValue::Private ) )
                return;

            QSettings    sett;
            sett.setValue( key, value );
            sett.sync();
            return ;
        }
    }
    vlmcWarning() << "Setting" << key << "does not exist.";
    Q_ASSERT_X( false, __FILE__, "set value without a created variable" );
}

SettingValue*
SettingsManager::value( const QString &key,
                        SettingsManager::Type type )
{
    QReadLocker rl( &m_rwLock );

    if ( type == Project )
    {
        SettingList::iterator   it = getPair( m_xmlSettings, key );
        if ( it != m_xmlSettings.end() )
            return (*it).value;
    }
    else
    {
        SettingList::iterator   it = getPair( m_classicSettings, key );
        if ( it != m_classicSettings.end() )
            return (*it).value;
    }
    vlmcWarning() << "Setting" << key << "does not exist.";
    Q_ASSERT_X( false, __FILE__, "get value without a created variable" );
    return NULL;
}

SettingsManager::SettingList
SettingsManager::group( const QString &groupName, SettingsManager::Type type )
{
    SettingsManager::SettingList        ret;
    QReadLocker                         rl( &m_rwLock );

    QString grp = groupName + '/';
    if ( type == Project )
    {
         SettingList::const_iterator it = m_xmlSettings.begin();
         SettingList::const_iterator ed = m_xmlSettings.end();

         for ( ; it != ed; ++it )
         {
             if ( (*it).key.startsWith( grp ) )
                 ret.push_back( *it );
         }
    }
    else if ( type == Vlmc )
    {
         SettingList::const_iterator it = m_classicSettings.begin();
         SettingList::const_iterator ed = m_classicSettings.end();

         for ( ; it != ed; ++it )
         {
             if ( (*it).key.startsWith( grp ) )
                 ret.push_back( *it );
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

    if ( type == Project )
    {
        SettingList::iterator   it = getPair( m_xmlSettings, key );
        if ( it != m_xmlSettings.end() )
        {
            connect( (*it).value, SIGNAL( changed( const QVariant& ) ),
                     receiver, method );
            return true;
        }
    }
    else if ( type == Vlmc )
    {
        SettingList::iterator   it = getPair( m_classicSettings, key );
        if ( it != m_classicSettings.end() )
        {
            connect( (*it).value, SIGNAL( changed( const QVariant& ) ),
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
    QReadLocker     rl( &m_rwLock );
    QSettings       sett;
    SettingList::const_iterator it = m_classicSettings.begin();
    SettingList::const_iterator ed = m_classicSettings.end();

    for ( ; it != ed; ++it )
    {
        if ( ( (*it).value->flags() & SettingValue::Private ) != 0 )
            continue ;
        sett.setValue( (*it).key, (*it).value->get() );
    }
    sett.sync();
}

void
SettingsManager::save( QXmlStreamWriter& project ) const
{
    SettingList::const_iterator     it = m_xmlSettings.begin();
    SettingList::const_iterator     end = m_xmlSettings.end();

    project.writeStartElement( "project" );
    for ( ; it != end; ++it )
    {
        if ( ( (*it).value->flags() & SettingValue::Private ) != 0 )
            continue ;
        project.writeStartElement( "property" );
        project.writeAttribute( "key", (*it).key );
        project.writeAttribute( "value", (*it).value->get().toString() );
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
    QWriteLocker    wLock( &m_rwLock );
    QDomElement s = element.firstChildElement();
    while ( s.isNull() == false )
    {
        QString     key = s.attribute( "key" );
        QString     value = s.attribute( "value" );

        if ( key.isEmpty() == true || value.isEmpty() == true )
            vlmcWarning() << "Invalid setting node.";
        else
        {
            if ( contains( m_xmlSettings, key ) == true )
                setValue( key, value, SettingsManager::Project );
            else
                vlmcWarning() << "Invalid setting node in project file:" << key;
        }
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
    QWriteLocker    wlock( &m_rwLock );

    SettingValue    *val = NULL;
    if ( varType == Vlmc && getPair( m_classicSettings, key ) == m_classicSettings.end() )
    {
        val = new SettingValue( type, defaultValue, name, desc, flags );
        m_classicSettings.push_back( Pair( key, val ) );
    }
    else if ( varType == Project && getPair( m_xmlSettings, key ) == m_xmlSettings.end() )
    {
        val = new SettingValue( type, defaultValue, name, desc, flags );
        m_xmlSettings.push_back( Pair( key, val ) );
    }
    else
        Q_ASSERT_X( false, __FILE__, "creating an already created variable" );
    return val;
}

SettingsManager::Pair::Pair( const QString &_key, SettingValue *_value ) :
        key( _key ),
        value( _value )
{
}

bool
SettingsManager::Pair::operator ==( const QString &_key ) const
{
    return key == _key;
}
