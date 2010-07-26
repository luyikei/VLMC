/*****************************************************************************
 * SettingsManager.h: Backend settings manager
 *****************************************************************************
 * Copyright (C) 2008-2009 VideoLAN
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

#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include "SettingValue.h"
#include "Singleton.hpp"

#include <QString>
#include <QHash>
#include <QObject>
#include <QReadWriteLock>
#include <QVariant>
#include <QXmlStreamWriter>

class SettingValue;
class QDomElement;


//Var helpers :
#define VLMC_GET_STRING( key )  SettingsManager::getInstance()->value( key )->get().toString()
#define VLMC_GET_INT( key )     SettingsManager::getInstance()->value( key )->get().toInt()
#define VLMC_GET_UINT( key )    SettingsManager::getInstance()->value( key )->get().toUInt()
#define VLMC_GET_DOUBLE( key )  SettingsManager::getInstance()->value( key )->get().toDouble()
#define VLMC_GET_BOOL( key )    SettingsManager::getInstance()->value( key )->get().toBool()

#define VLMC_PROJECT_GET_STRING( key )  SettingsManager::getInstance()->value( key, SettingsManager::Project )->get().toString()
#define VLMC_PROJECT_GET_INT( key )     SettingsManager::getInstance()->value( key, SettingsManager::Project )->get().toInt()
#define VLMC_PROJECT_GET_UINT( key )    SettingsManager::getInstance()->value( key, SettingsManager::Project )->get().toUInt()
#define VLMC_PROJECT_GET_DOUBLE( key )  SettingsManager::getInstance()->value( key, SettingsManager::Project )->get().toDouble()
#define VLMC_PROJECT_GET_BOOL( key )    SettingsManager::getInstance()->value( key, SettingsManager::Project )->get().toBool()


#define VLMC_CREATE_PROJECT_VAR( type, key, defaultValue, name, desc, flags )  \
SettingsManager::getInstance()->createVar( type, key, defaultValue, name, \
                                           desc, SettingsManager::Project, flags );

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
SettingsManager::getInstance()->createVar( type, key, defaultValue, name,  \
                                           desc, SettingsManager::Vlmc, flags );

/// Vlmc preferences maccros
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

//Convenience maccros :
#define VLMC_CREATE_PRIVATE_PREFERENCE_STRING( key, defaultValue )  \
        VLMC_CREATE_PREFERENCE( SettingValue::String, key, defaultValue, "", "", SettingValue::Private )
#define VLMC_CREATE_PRIVATE_PROJECT_STRING( key, defaultValue )  \
        VLMC_CREATE_PROJECT_VAR( SettingValue::String, key, defaultValue, "", "", SettingValue::Private )
#define VLMC_CREATE_PREFERENCE_PASSWORD( key, defaultValue, name, desc )  \
        VLMC_CREATE_PREFERENCE( SettingValue::String, key, defaultValue, name, desc, SettingValue::Password )


class   SettingsManager : public QObject, public Singleton<SettingsManager>
{
    Q_OBJECT
    Q_DISABLE_COPY( SettingsManager )
    public:
        enum Type
        {
            Project,
            Vlmc
        };

        struct  Pair
        {
            Pair( const QString& key, SettingValue *value );
            bool            operator==( const QString& key ) const;
            QString         key;
            SettingValue    *value;
        };
        //We store this as a list to preserve order.
        typedef QList<Pair>         SettingList;

        void                        setValue( const QString &key,
                                                const QVariant &value,
                                                SettingsManager::Type type = Vlmc);
        SettingValue                *value( const QString &key,
                                            SettingsManager::Type type = Vlmc );
        SettingList                 group( const QString &groupName,
                                            SettingsManager::Type type = Vlmc );

        SettingValue                *createVar( SettingValue::Type type, const QString &key,
                                               const QVariant &defaultValue,
                                               const char *name, const char *desc,
                                               Type varType = Vlmc,
                                               SettingValue::Flags flags = SettingValue::Nothing );
        bool                        watchValue( const QString &key,
                                                QObject* receiver,
                                                const char *method,
                                                SettingsManager::Type type,
                                                Qt::ConnectionType cType = Qt::AutoConnection );
        void                        save() const;
        void                        save( QXmlStreamWriter& project ) const;
        bool                        load( const QDomElement &element );

    private:
        friend class Singleton<SettingsManager>;
        SettingsManager(){}
        ~SettingsManager(){}

        SettingList                 m_classicSettings;
        SettingList                 m_xmlSettings;

        mutable QReadWriteLock      m_rwLock;
};

#endif
