/*****************************************************************************
 * Core.cpp: VLMC Base functions.
 *****************************************************************************
 * Copyright (C) 2008-2014 the VLMC team
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

#include "Core.h"

#include <QtGlobal>
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
# include <QStandardPaths>
#else
# include <QDesktopServices>
#endif

#include <Backend/IBackend.h>
#include <EffectsEngine/EffectsEngine.h>
#include <Settings/Settings.h>
#include <Tools/VlmcLogger.h>

Core::Core()
{
    m_backend = Backend::getBackend();
    m_effectsEngine = new EffectsEngine;
    m_logger = new VlmcLogger;
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    m_settings = new Settings( QStandardPaths::writableLocation( QStandardPaths::ConfigLocation ) );
#else
    m_settings = new Settings( QDesktopServices::storageLocation( QDesktopServices::DataLocation ) );
#endif
}

Core::~Core()
{
    delete m_settings;
    delete m_logger;
    delete m_effectsEngine;
    delete m_backend;
}

Backend::IBackend*
Core::backend()
{
    return m_backend;
}

EffectsEngine*
Core::effectsEngine()
{
    return m_effectsEngine;
}

VlmcLogger*
Core::logger()
{
    return m_logger;
}

Settings*
Core::settings()
{
    return m_settings;
}
