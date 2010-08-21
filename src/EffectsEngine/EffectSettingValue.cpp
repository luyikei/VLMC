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

#include <QColor>
#include <QPoint>
#include <QtDebug>

EffectSettingValue::EffectSettingValue( Type type, EffectInstance* instance, quint32 index,
                                        const char *name, const char *desc, Flags flags ) :
    SettingValue( type, QVariant(), name, desc, flags ),
    m_paramBuff( NULL ),
    m_buffSize( 0 ),
    m_effectInstance( instance ),
    m_index( index )
{
    //Fetch the default value.
    m_defaultVal = get();
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
    case String:
        {
            QByteArray      bytes = val.toString().toUtf8();
            const char*   tmp = bytes;
            copyToFrei0rBuff( tmp, bytes.length() );
            break ;
        }
    case Bool:
        {
            bool    tmp = val.toBool();
            copyToFrei0rBuff( &tmp );
            break ;
        }
    case Color:
        {
            QColor  color = val.value<QColor>();
            float   rgb[3] = { color.redF(), color.greenF(), color.blueF() };
            copyToFrei0rBuff( rgb, 3 * sizeof(float) );
            break ;
        }
    case Position:
        {
            QPointF     pos = val.value<QPointF>();
            double      posD[2] = { pos.x(), pos.y() };
            copyToFrei0rBuff( posD, 2 * sizeof(double) );
            break ;
        }
    default:
        qCritical() << "Setting type" << m_type << "is not handled by the effects engine";
        break;
    }
    apply();
}

void
EffectSettingValue::apply()
{
    if ( m_paramBuff != NULL )
        m_effectInstance->effect()->m_f0r_set_param_value( m_effectInstance->m_instance,
                                                           m_paramBuff, m_index );
}

const QVariant&
EffectSettingValue::get()
{
    switch ( m_type )
    {
    case Double:
        {
            double  tmp;
            m_effectInstance->effect()->m_f0r_get_param_value( m_effectInstance->m_instance,
                                                               &tmp, m_index );
            m_val = tmp;
            break ;
        }
    case String:
        {
            char    *tmp;
            m_effectInstance->effect()->m_f0r_get_param_value( m_effectInstance->m_instance,
                                                               &tmp, m_index );
            m_val = QString::fromUtf8( tmp );
            break ;
        }
    case Bool:
        {
            bool    tmp;
            m_effectInstance->effect()->m_f0r_get_param_value( m_effectInstance->m_instance,
                                                               &tmp, m_index );
            m_val = tmp;
            break ;
        }
    case Color:
        {
            f0r_param_color_t   tmp;
            m_effectInstance->effect()->m_f0r_get_param_value( m_effectInstance->m_instance,
                                                               &tmp, m_index );
            m_val = QColor( tmp.r, tmp.g, tmp.b );
            break ;
        }
    case Position:
        {
            f0r_param_position_t    tmp;
            m_effectInstance->effect()->m_f0r_get_param_value( m_effectInstance->m_instance,
                                                               &tmp, m_index );
            m_val = QPointF( tmp.x, tmp.y );
            break ;
        }
    default:
        qCritical() << "Setting type" << m_type << "is not handled by the effects engine";
        m_val = QVariant();
    }
    return m_val;
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
