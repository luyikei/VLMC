/*****************************************************************************
 * Settings.h: Backend settings manager
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

#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include "Main/Core.h"
#include "Project/Project.h"
#include "SettingValue.h"

#include <QString>
#include <QMap>
#include <QObject>
#include <QReadWriteLock>
#include <QVariant>
#include <QXmlStreamWriter>

class SettingValue;

class QFile;
class QDomElement;


//Var helpers :
#define VLMC_GET_STRING( key )      Core::getInstance()->settings()->value( key )->get().toString()
#define VLMC_GET_INT( key )         Core::getInstance()->settings()->value( key )->get().toInt()
#define VLMC_GET_UINT( key )        Core::getInstance()->settings()->value( key )->get().toUInt()
#define VLMC_GET_DOUBLE( key )      Core::getInstance()->settings()->value( key )->get().toDouble()
#define VLMC_GET_BOOL( key )        Core::getInstance()->settings()->value( key )->get().toBool()
#define VLMC_GET_STRINGLIST( key )  Core::getInstance()->settings()->value( key )->get().toStringList()
#define VLMC_GET_BYTEARRAY( key )   Core::getInstance()->settings()->value( key )->get().toByteArray()

#define VLMC_PROJECT_GET_STRING( key )  Project::getInstance()->settings()->value( key )->get().toString()
#define VLMC_PROJECT_GET_INT( key )     Project::getInstance()->settings()->value( key )->get().toInt()
#define VLMC_PROJECT_GET_UINT( key )    Project::getInstance()->settings()->value( key )->get().toUInt()
#define VLMC_PROJECT_GET_DOUBLE( key )  Project::getInstance()->settings()->value( key )->get().toDouble()
#define VLMC_PROJECT_GET_BOOL( key )    Project::getInstance()->settings()->value( key )->get().toBool()


#define VLMC_CREATE_PROJECT_VAR( type, key, defaultValue, name, desc, flags )  \
        Project::getInstance()->settings()->createVar( type, key, defaultValue, name, \
                                                       desc, flags );

#define VLMC_CREATE_PROJECT_INT( key, defaultValue, name, desc )  \
        VLMC_CREATE_PROJECT_VAR( SettingValue::Int, key, defaultValue, name, desc, SettingValue::Nothing )
#define VLMC_CREATE_PROJECT_STRING( key, defaultValue, name, desc )  \
        VLMC_CREATE_PROJECT_VAR( SettingValue::String, key, defaultValue, name, desc, SettingValue::Nothing )
#define VLMC_CREATE_PROJECT_DOUBLE( key, defaultValue, name, desc )  \
        VLMC_CREATE_PROJECT_VAR( SettingValue::Double, key, defaultValue, name, desc, SettingValue::Nothing )
#define VLMC_CREATE_PROJECT_BOOL( key, defaultValue, name, desc )  \
        VLMC_CREATE_PROJECT_VAR( SettingValue::Bool, key, defaultValue, name, desc, SettingValue::Nothing )
#define VLMC_CREATE_PROJECT_PATH( key, defaultValue, name, desc )  \
        VLMC_CREATE_PROJECT_PATH( SettingValue::Path, key, defaultValue, name, desc, SettingValue::Nothing )


#define VLMC_CREATE_PREFERENCE( type, key, defaultValue, name, desc, flags )  \
        Core::getInstance()->settings()->createVar( type, key, defaultValue, name,  \
                                                       desc, flags );

/// Vlmc preferences macros
#define VLMC_CREATE_PREFERENCE_INT( key, defaultValue, name, desc )  \
        VLMC_CREATE_PREFERENCE( SettingValue::Int, key, defaultValue, name, desc, SettingValue::Nothing )
#define VLMC_CREATE_PREFERENCE_STRING( key, defaultValue, name, desc )  \
        VLMC_CREATE_PREFERENCE( SettingValue::String, key, defaultValue, name, desc, SettingValue::Nothing )
#define VLMC_CREATE_PREFERENCE_DOUBLE( key, defaultValue, name, desc )  \
        VLMC_CREATE_PREFERENCE( SettingValue::Double, key, defaultValue, name, desc, SettingValue::Nothing )
#define VLMC_CREATE_PREFERENCE_LANGUAGE( key, defaultValue, name, desc )  \
        VLMC_CREATE_PREFERENCE( SettingValue::Language, key, defaultValue, name, desc, SettingValue::Nothing )
#define VLMC_CREATE_PREFERENCE_KEYBOARD( key, defaultValue, name, desc )  \
        VLMC_CREATE_PREFERENCE( SettingValue::KeyboardShortcut, key, defaultValue, name, desc, SettingValue::Nothing )
#define VLMC_CREATE_PREFERENCE_BOOL( key, defaultValue, name, desc )  \
        VLMC_CREATE_PREFERENCE( SettingValue::Bool, key, defaultValue, name, desc, SettingValue::Nothing )
#define VLMC_CREATE_PREFERENCE_PATH( key, defaultValue, name, desc )  \
        VLMC_CREATE_PREFERENCE( SettingValue::Path, key, defaultValue, name, desc, SettingValue::Nothing )

/// Convenience macros :
#define VLMC_CREATE_PRIVATE_PREFERENCE_STRING( key, defaultValue )  \
        VLMC_CREATE_PREFERENCE( SettingValue::String, key, defaultValue, "", "", SettingValue::Private )
#define VLMC_CREATE_PRIVATE_PREFERENCE_INT( key, defaultValue )  \
        VLMC_CREATE_PREFERENCE( SettingValue::Int, key, defaultValue, "", "", SettingValue::Private )
#define VLMC_CREATE_PRIVATE_PREFERENCE_BOOL( key, defaultValue )  \
        VLMC_CREATE_PREFERENCE( SettingValue::Bool, key, defaultValue, "", "", SettingValue::Private )
#define VLMC_CREATE_PRIVATE_PREFERENCE_BYTEARRAY( key, defaultValue )  \
        VLMC_CREATE_PREFERENCE( SettingValue::ByteArray, key, defaultValue, "", "", SettingValue::Private )

#define VLMC_CREATE_PREFERENCE_PASSWORD( key, defaultValue, name, desc )  \
        VLMC_CREATE_PREFERENCE( SettingValue::String, key, defaultValue, name, desc, SettingValue::Password )
#define VLMC_CREATE_PRIVATE_PROJECT_STRING( key, defaultValue )  \
        VLMC_CREATE_PROJECT_VAR( SettingValue::String, key, defaultValue, "", "", SettingValue::Private )


class   Settings
{
    public:
        typedef QList<SettingValue*>                SettingList;
        typedef QMap<QString, SettingValue*>        SettingMap;

        Settings( const QString& settingsFile );
        ~Settings();
        bool                        setValue(const QString &key, const QVariant &value );
        SettingValue*               value( const QString &key );
        SettingValue*               createVar( SettingValue::Type type, const QString &key, const QVariant &defaultValue, const char *name, const char *desc, SettingValue::Flags flags );
        SettingList                 group( const QString &groupName ) const;
        bool                        load();
        void                        save() const;
        void                        save( QXmlStreamWriter& project ) const;
        bool                        watchValue( const QString &key, QObject* receiver, const char *method, Qt::ConnectionType cType = Qt::AutoConnection );
        void                        setSettingsFile( const QString& settingsFile );
    private:
        SettingMap                  m_settings;
        mutable QReadWriteLock      m_rwLock;
        QFile*                      m_settingsFile;
};

#endif
