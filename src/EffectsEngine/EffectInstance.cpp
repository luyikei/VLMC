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

#include "Effect.h"

EffectInstance::EffectInstance( Effect *effect ) :
        m_effect( effect ),
        m_width( 0 ),
        m_height( 0 ),
        m_instance( NULL )
{
}

EffectInstance::~EffectInstance()
{
    m_effect->m_f0r_destruct( m_instance );
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
    }
}

void
EffectInstance::process( double time, const quint32 *input, quint32 *output ) const
{
    m_effect->m_f0r_update( m_instance, time, input, output );
}

Effect*
EffectInstance::effect()
{
    return m_effect;
}
