/*****************************************************************************
 * Effect.cpp: Handle a frei0r effect.
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

#include "Effect.h"

#include "frei0r/frei0r.h"

#include <QtDebug>

Effect::Effect( const QString &fileName ) :
        QLibrary( fileName ),
        m_instance( NULL ),
        m_width( 0 ),
        m_height( 0 )
{
}

Effect::~Effect()
{
    if ( m_instance != NULL )
    {
        m_f0r_deinit();
        m_f0r_destruct( m_instance );
    }
}

#define LOAD_FREI0R_SYMBOL( dest, symbolName )  \
if ( ( dest = reinterpret_cast<typeof( dest )>( resolve( symbolName ) ) ) == NULL ) \
{                                                                       \
    qCritical() << "Failed to load symbol:" << symbolName;              \
    return false;                                                       \
}

bool
Effect::load()
{
    LOAD_FREI0R_SYMBOL( m_f0r_init, "f0r_init" );
    LOAD_FREI0R_SYMBOL( m_f0r_deinit, "f0r_deinit" )
    LOAD_FREI0R_SYMBOL( m_f0r_info, "f0r_get_plugin_info" )
    LOAD_FREI0R_SYMBOL( m_f0r_construct, "f0r_construct" )
    LOAD_FREI0R_SYMBOL( m_f0r_destruct, "f0r_destruct" )
    LOAD_FREI0R_SYMBOL( m_f0r_update, "f0r_update" )

    //Initializing structures
    f0r_plugin_info_t   infos;

    m_f0r_info( &infos );
    m_name = infos.name;
    m_desc = infos.explanation;
    m_type = static_cast<Type>( infos.plugin_type );
    return true;
}

#undef LOAD_FREI0R_SYMBOL

const QString&
Effect::name() const
{
    return m_name;
}

const QString&
Effect::description() const
{
    return m_desc;
}

Effect::Type
Effect::type() const
{
    return m_type;
}

void
Effect::init( quint32 width, quint32 height )
{
    //Don't init if the effect is not currently used.
    if ( m_used == false )
        return ;
    if ( width != m_width || height != m_height )
    {
        if ( m_instance != NULL )
        {
            m_f0r_deinit();
            m_f0r_destruct( m_instance );
        }
        m_instance = m_f0r_construct( width, height );
        m_f0r_init();
        m_width = width;
        m_height = height;
    }
}

void
Effect::process( double time, const quint32 *input, quint32 *output ) const
{
    m_f0r_update( m_instance, time, input, output );
}

void
Effect::setUsed( bool used )
{
    m_used = used;
}
