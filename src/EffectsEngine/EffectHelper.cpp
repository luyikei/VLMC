/*****************************************************************************
 * EffectHelper: Contains informations about effects
 *****************************************************************************
 * Copyright (C) 2008-2010 VideoLAN
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

#include "EffectHelper.h"
#include "EffectUser.h"
#include "MainWorkflow.h"

EffectHelper::EffectHelper( EffectInstance *effectInstance, qint64 begin, qint64 end,
                            const QString &uuid ) :
    Helper( begin, end, uuid ),
    m_effectInstance( effectInstance ),
    m_target( NULL )
{
    if ( MainWorkflow::getInstance()->getLengthFrame() > 0 )
        m_end = MainWorkflow::getInstance()->getLengthFrame();
    else
        m_end = Effect::TrackEffectDefaultLength;
}

EffectInstance*
EffectHelper::effectInstance()
{
    return m_effectInstance;
}

const EffectInstance*
EffectHelper::effectInstance() const
{
    return m_effectInstance;
}

EffectUser*
EffectHelper::target()
{
    return m_target;
}

void
EffectHelper::setTarget( EffectUser *target )
{
    m_target = target;
    if ( target != NULL )
    {
        if ( target->length() > 0 && target->length() < m_end )
            m_end = target->length();
    }
}
