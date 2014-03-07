/*****************************************************************************
 * Core.h: VLMC Base functions.
 *****************************************************************************
 * Copyright (C) 2008-2014 the VLMC team
 *
 * Authors: Ludovic Fauvet <etix@l0cal.com>
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

#ifndef CORE_H
#define CORE_H

class EffectsEngine;
class NotificationZone;
class SettingsManager;
class VlmcLogger;

namespace Backend
{
    class IBackend;
}

#include <Tools/Singleton.hpp>

class Core : public Singleton<Core>
{
    public:
        Backend::IBackend*      backend();
        EffectsEngine*          effectsEngine();
        SettingsManager*        settings();
        VlmcLogger*             logger();

    private:
        Core();
        ~Core();
        Backend::IBackend*      m_backend;
        EffectsEngine*          m_effectsEngine;
        //FIXME: This should only be applications settings.
        SettingsManager*        m_settings;
        VlmcLogger*             m_logger;

        friend class Singleton<Core>;
};

#endif // CORE_H
