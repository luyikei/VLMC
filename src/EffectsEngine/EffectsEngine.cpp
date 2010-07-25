/*****************************************************************************
 * EffectsEngine.cpp: Manage the effects plugins.
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

#include "EffectsEngine.h"

#include "Effect.h"

#include <QDir>

EffectsEngine::EffectsEngine()
{
    Effect  *e = new Effect("libbw0r");

    m_effects.push_back( e );
}

EffectsEngine::~EffectsEngine()
{
}

void
EffectsEngine::initAll( quint32 width, quint32 height )
{
    foreach ( Effect *e, m_effects )
        e->init( width, height );
}

Effect*
EffectsEngine::effect( qint32 idx )
{
    return m_effects.at( idx );
}

bool
EffectsEngine::loadEffect( const QString &fileName )
{
    Effect*     e = new Effect( fileName );
    if ( e->load() == false )
    {
        delete e;
        return false;
    }
    m_effects.push_back( e );
    emit effectAdded( e, e->type() );
    return true;
}

void
EffectsEngine::browseDirectory( const QString &path )
{
    QDir    dir( path );
    const QStringList& files = dir.entryList( QDir::Files | QDir::NoDotAndDotDot |
                                              QDir::Readable | QDir::Executable );
    foreach ( const QString& file, files )
    {
        loadEffect( path + '/' + file );
    }
}
