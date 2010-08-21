/*****************************************************************************
 * EffectInstance.cpp: Handle an effect instance.
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

#include "EffectInstance.h"
#include "EffectSettingValue.h"

#include "Effect.h"

#include <QtDebug>

EffectInstance::EffectInstance( Effect *effect ) :
        m_effect( effect ),
        m_width( 0 ),
        m_height( 0 ),
        m_instance( NULL )
{
    Effect::ParamList::const_iterator       it = effect->params().constBegin();
    Effect::ParamList::const_iterator       ite = effect->params().constEnd();
    quint32                                 i = 0;

    while ( it != ite )
    {
        f0r_param_info_t    *info = *it;
        m_params[info->name] = settingValueFactory( info, i );
        ++it;
        ++i;
    }
}

EffectInstance::~EffectInstance()
{
    m_effect->m_f0r_destruct( m_instance );
}

EffectSettingValue*
EffectInstance::settingValueFactory( f0r_param_info_t *info, quint32 index )
{
    SettingValue::Flag      flags = SettingValue::Nothing;

    if ( info->type == F0R_PARAM_DOUBLE )
        flags = SettingValue::Clamped;
    EffectSettingValue  *val = new EffectSettingValue( EffectSettingValue::frei0rToVlmc( info->type ),
                                                        this, index, QVariant(),
                                                        info->name, info->explanation );
    if ( info->type == F0R_PARAM_DOUBLE )
        val->setLimits( 0.0, 1.0 );
    return val;
}

void
EffectInstance::init( quint32 width, quint32 height )
{
    if ( width != m_width || height != m_height )
    {
        m_effect->load();
        m_instance = m_effect->m_f0r_construct( width, height );
        m_width = width;
        m_height = height;
        //Re-apply parameters, as they were lost during f0r_destruct();
        foreach ( EffectSettingValue* val, m_params.values() )
            val->apply();
    }
}

Effect*
EffectInstance::effect()
{
    return m_effect;
}

const EffectInstance::ParamList&
EffectInstance::params() const
{
    return m_params;
}

EffectInstance::ParamList&
EffectInstance::params()
{
    return m_params;
}
