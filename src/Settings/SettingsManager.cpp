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

void
SettingsManager::setValue( const QString &key,
                           const QVariant &value,
                           SettingsManager::Type type )
{
    if ( type == Project )
    {
        SettingMap::iterator   it = m_xmlSettings.find( key );
        if ( it != m_xmlSettings.end() )
        {
            (*it)->set( value );
            return ;
        }
    }
    else// ( type == Vlmc && m_classicSettings.contains( key) == true )
    {
        SettingMap::iterator   it = m_classicSettings.find( key );
        if ( it != m_classicSettings.end() )
        {
            SettingValue* v = (*it);

            // We don't want private values in our QSettings, that would be
            // saved in the preference files, and they're called private for a reason
            // FIXME: For now we have only one private variable which is for runtime stuff
            // (logging level) We might want to split this in Private & Runtime at some point.
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
        SettingMap::iterator   it = m_xmlSettings.find( key );
        if ( it != m_xmlSettings.end() )
            return (*it);
    }
    else
    {
        SettingMap::iterator   it = m_classicSettings.find( key );
        if ( it != m_classicSettings.end() )
            return (*it);
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
         SettingMap::const_iterator it = m_xmlSettings.begin();
         SettingMap::const_iterator ed = m_xmlSettings.end();

         for ( ; it != ed; ++it )
         {
             if ( (*it)->key().startsWith( grp ) )
                 ret.push_back( *it );
         }
    }
    else if ( type == Vlmc )
    {
         SettingMap::const_iterator it = m_classicSettings.begin();
         SettingMap::const_iterator ed = m_classicSettings.end();

         for ( ; it != ed; ++it )
         {
             if ( (*it)->key().startsWith( grp ) )
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
        SettingMap::iterator   it = m_xmlSettings.find( key );
        if ( it != m_xmlSettings.end() )
        {
            connect( (*it), SIGNAL( changed( const QVariant& ) ),
                     receiver, method );
            return true;
        }
    }
    else if ( type == Vlmc )
    {
        SettingMap::iterator   it = m_classicSettings.find( key );
        if ( it != m_classicSettings.end() )
        {
            connect( (*it), SIGNAL( changed( const QVariant& ) ),
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
    SettingMap::const_iterator it = m_classicSettings.begin();
    SettingMap::const_iterator ed = m_classicSettings.end();

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
    SettingMap::const_iterator     it = m_xmlSettings.begin();
    SettingMap::const_iterator     end = m_xmlSettings.end();

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
            if ( m_xmlSettings.contains( key ) == true )
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
    if ( varType == Vlmc && m_classicSettings.contains( key ) == false )
    {
        val = new SettingValue( key, type, defaultValue, name, desc, flags );
        m_classicSettings.insert( key, val );
    }
    else if ( varType == Project && m_xmlSettings.contains( key ) == false )
    {
        val = new SettingValue( key, type, defaultValue, name, desc, flags );
        m_xmlSettings.insert( key, val );
    }
    else
        Q_ASSERT_X( false, __FILE__, "creating an already created variable" );
    return val;
}
