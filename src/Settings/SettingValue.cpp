/*****************************************************************************
 * SettingValue.cpp: A setting value that can broadcast its changes
 *****************************************************************************
 * Copyright (C) 2008-2016 VideoLAN
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "SettingValue.h"

SettingValue::SettingValue( const QString& key, SettingValue::Type type, const QVariant& defaultValue,
                            const char* name, const char* desc, SettingValue::Flags flags ) :
        m_val( defaultValue ),
        m_defaultVal( defaultValue ),
        m_key( key ),
        m_name( name ),
        m_desc( desc ),
        m_type( type ),
        m_flags( flags ),
        m_initLoad( true )
{
}

void
SettingValue::set( const QVariant& _val )
{
    QVariant val = _val;
    if ( val != m_val )
    {
        if ( ( m_flags & Clamped ) != 0 )
        {
            if ( m_min.isValid() && val.toDouble() < m_min.toDouble() )
                val = m_min;
            if ( m_max.isValid() && val.toDouble() > m_max.toDouble() )
                val = m_max;
        }
        if ( ( m_flags & EightMultiple ) != 0 )
            val = ( val.toInt() + 7 ) & ~7;
        m_val = val;
        emit changed( m_val );
    }
    else if ( m_initLoad )
        emit changed( m_val );
    m_initLoad = false;
}

const QVariant&
SettingValue::get()
{
    return m_val;
}

const char*
SettingValue::description() const
{
    return m_desc;
}

void
SettingValue::restoreDefault()
{
    set( m_defaultVal );
}

const QString&
SettingValue::key() const
{
    return m_key;
}

const char*
SettingValue::name() const
{
    return m_name;
}

SettingValue::Type
SettingValue::type() const
{
    return m_type;
}

SettingValue::Flags
SettingValue::flags() const
{
    return m_flags;
}

void
SettingValue::setLimits( const QVariant& min, const QVariant& max )
{
    Q_ASSERT_X( ( ( m_flags & Clamped ) != 0 ), "SettingValue", "Setting limits to a non-clamped value" );
    if ( min.isValid() == true )
        m_min = min;
    if ( max.isValid() == true )
        m_max = max;
}

const QVariant&
SettingValue::min() const
{
    return m_min;
}

const QVariant&
SettingValue::max() const
{
    return m_max;
}
