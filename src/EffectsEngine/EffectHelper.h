/*****************************************************************************
 * EffectHelper: Contains informations about effects
 *****************************************************************************
 * Copyright (C) 2008-2016 VideoLAN
 *
 * Authors: Yikei Lu    <luyikei.qmltu@gmail.com>
 *          Hugo Beauzée-Luyssen <hugo@beauzee.fr>
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

#ifndef EFFECTHELPER_H
#define EFFECTHELPER_H

#include <QObject>
#include <QMetaType>

#include "Settings/Settings.h"
#include "Backend/IFilter.h"
#include "Workflow/Helper.h"

namespace Backend
{
namespace MLT
{
class MLTFilter;
}
class IFilter;
class IService;
}

class Effect;

class   EffectHelper : public Workflow::Helper
{
    Q_OBJECT

    public:
        EffectHelper( const char* id, qint64 begin = 0, qint64 end = -1,
                      const QString& uuid = QString() );
        EffectHelper( const QString& id, qint64 begin = 0, qint64 end = -1,
                      const QString& uuid = QString() );
        EffectHelper( Backend::IFilter* filter,
                      const QString& uuid = QString() );
        EffectHelper( const QVariant& variant );
        ~EffectHelper();

        virtual qint64  begin() const override;
        virtual qint64  end() const override;
        virtual void    setBegin(qint64 begin) override;
        virtual void    setEnd(qint64 end) override;
        virtual qint64  length() const override;
        virtual void    setBoundaries( qint64 begin, qint64 end ) override;

        void                setTarget( Backend::IService* service );
        Backend::IService*  target();

        Backend::IFilterInfo*           filterInfo();

        QString                         identifier();

        Backend::IFilter*               filter();
        const Backend::IFilter*         filter() const;

        SettingValue*                   value( const QString& key );

        // Handle one filter.
        void                            loadFromVariant( const QVariant& variant );
        QVariant                        toVariant();

        // Handle one service
        static QVariant                 toVariant( Backend::IService* service );
        static void                     loadFromVariant( const QVariant& variant, Backend::IService* service );

    private:
        Backend::MLT::MLTFilter*    m_filter;
        Backend::IService*          m_service;
        Backend::IFilterInfo*       m_filterInfo;

        Settings                    m_settings;

        void                        set( SettingValue* value, const QVariant& variant );
        QVariant                    defaultValue( const char* id, SettingValue::Type type );
        void                        initParams();
};

Q_DECLARE_METATYPE( Backend::IFilter* );

#endif // EFFECTHELPER_H
