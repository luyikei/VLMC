/*****************************************************************************
 * EffectSettingValue.h: Handle an effect instance.
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

#ifndef EFFECTSETTINGVALUE_H
#define EFFECTSETTINGVALUE_H

#include "SettingValue.h"
#include "frei0r.h"

class   EffectInstance;

class EffectSettingValue : public SettingValue
{
    Q_OBJECT

    public:
        EffectSettingValue( Type type, EffectInstance* instance, quint32 index,
                            const QVariant& defaultValue, const char* name,
                            const char* desc, Flags flags = Nothing );

        f0r_param_t     getFrei0rParameter() const;
        virtual void    set( const QVariant& val );
        quint32         index() const;
        static Type     frei0rToVlmc( int type );

    private:
        template <typename T>
        void            copyToFrei0rBuff( const T* ptr, quint32 size = sizeof( T ) )
        {
            if ( m_buffSize != size || m_paramBuff == NULL )
            {
                delete[] m_paramBuff; //Won't hurt if paramBuff is NULL
                m_paramBuff = new qint8[size];
            }
            memcpy( m_paramBuff, ptr, size );
        }

    private:
        qint8           *m_paramBuff;
        quint32         m_buffSize;
        EffectInstance  *m_instance;
        quint32         m_index;
};

#endif // EFFECTSETTINGVALUE_H
