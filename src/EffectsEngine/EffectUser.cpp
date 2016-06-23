/*****************************************************************************
 * EffectUser.cpp: Handles effects list and application
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

#include <QVariant>
#include <QReadWriteLock>

#include "EffectsEngine/EffectUser.h"
#include "EffectsEngine/EffectHelper.h"
#include "EffectsEngine/EffectInstance.h"

#include "Main/Core.h"
#include "Workflow/Types.h"
#include "Tools/VlmcDebug.h"

EffectUser::EffectUser() :
        m_isRendering( false ),
        m_width( 0 ),
        m_height( 0 )
{
    m_effectsLock = new QReadWriteLock();
}

EffectUser::~EffectUser()
{
    delete m_effectsLock;
}
