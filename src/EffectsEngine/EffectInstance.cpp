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
    init( 1, 1 );

    Effect::ParamList::const_iterator       it = effect->params().constBegin();
    Effect::ParamList::const_iterator       ite = effect->params().constEnd();
    quint32                                 i = 0;

    while ( it != ite )
    {
        Effect::Parameter   *info = *it;
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
EffectInstance::settingValueFactory( Effect::Parameter *info, quint32 index )
{
    SettingValue::Flag      flags = SettingValue::Nothing;

    if ( info->type == F0R_PARAM_DOUBLE )
        flags = SettingValue::Clamped;
    EffectSettingValue  *val = new EffectSettingValue( EffectSettingValue::frei0rToVlmc( info->type ),
                                                        this, index, info->name, info->desc );
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

bool
EffectInstance::isInit() const
{
    return m_instance != NULL;
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

void
EffectInstance::process( double time, const quint32 *frame1, const quint32 *frame2,
                       const quint32 *frame3, quint32 *output )
{
    Q_ASSERT( m_effect->type() == Effect::Mixer2 );
    m_effect->m_f0r_update2( m_instance, time, frame1, frame2, frame3, output );
}

void
EffectInstance::process( double time, const quint32 *input, quint32 *output ) const
{
    Q_ASSERT( m_effect->type() == Effect::Filter );
    m_effect->m_f0r_update( m_instance, time, input, output );
}
