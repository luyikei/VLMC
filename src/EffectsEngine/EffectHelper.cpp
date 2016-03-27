/*****************************************************************************
 * EffectHelper: Contains informations about effects
 *****************************************************************************
 * Copyright (C) 2008-2014 VideoLAN
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

#include "EffectsEngine/EffectHelper.h"
#include "EffectsEngine/EffectUser.h"
#include "Main/Core.h"
#include "Project/Project.h"
#include "Workflow/MainWorkflow.h"

EffectHelper::EffectHelper( EffectInstance *effectInstance, qint64 begin, qint64 end,
                            const QString &uuid ) :
    Helper( begin, end, uuid ),
    m_effectInstance( effectInstance ),
    m_target( nullptr )
{
    if ( Core::getInstance()->workflow()->getLengthFrame() > 0 )
        m_end = Core::getInstance()->workflow()->getLengthFrame();
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
    if ( target != nullptr )
    {
        if ( target->length() > 0 && target->length() < m_end )
            m_end = target->length();
    }
}
