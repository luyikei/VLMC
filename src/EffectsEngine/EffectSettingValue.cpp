/*****************************************************************************
 * EffectSettingValue.cpp: Handle an effect instance.
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

#include "Effect.h"
#include "EffectSettingValue.h"
#include "EffectInstance.h"

#include <QtDebug>

EffectSettingValue::EffectSettingValue( Type type, EffectInstance* instance, quint32 index,
                                        const QVariant &defaultValue, const char *name,
                                        const char *desc, Flags flags ) :
    SettingValue( type, defaultValue, name, desc, flags ),
    m_paramBuff( NULL ),
    m_buffSize( 0 ),
    m_effectInstance( instance ),
    m_index( index )
{
}

EffectSettingValue::~EffectSettingValue()
{
    delete[] m_paramBuff;
}

f0r_param_t
EffectSettingValue::getFrei0rParameter() const
{
    return reinterpret_cast<void*>( m_paramBuff );
}

void
EffectSettingValue::set( const QVariant &val )
{
    SettingValue::set( val );
    switch ( m_type )
    {
    case Double:
        {
            double  tmp = val.toDouble();
            copyToFrei0rBuff( &tmp );
            break ;
        }
//        case String:
//        case Bool:
//        case Color:
//        case Position:
    default:
        qCritical() << "Setting type" << m_type << "is not handled by the effects engine";
        break;
    }
    m_effectInstance->effect()->m_f0r_set_param_value( m_effectInstance->m_instance, m_paramBuff, m_index );
}

quint32
EffectSettingValue::index() const
{
    return m_index;
}

SettingValue::Type
EffectSettingValue::frei0rToVlmc( int type )
{
    switch ( type )
    {
    case F0R_PARAM_BOOL:
        return Bool;
    case F0R_PARAM_DOUBLE:
        return Double;
    case F0R_PARAM_COLOR:
        return Color;
    case F0R_PARAM_POSITION:
        return Position;
    case F0R_PARAM_STRING:
        return String;
    default:
        qCritical() << "Invalid effect setting type ! Undefined behaviour";
    }
    //Keeping compiler happy.
    return Double;
}
