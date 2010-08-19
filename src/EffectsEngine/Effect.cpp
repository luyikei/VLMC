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
#include "FilterInstance.h"

#include <QtDebug>

Effect::Effect( const QString &fileName ) :
        QLibrary( fileName ),
        m_type( Unknown ),
        m_major( -1 ),
        m_minor( -1 ),
        m_nbParams( -1 )
{
}

Effect::~Effect()
{
    m_f0r_deinit();
    qDeleteAll( m_params );
    m_params.clear();
}

#define LOAD_FREI0R_SYMBOL( dest, symbolName )  \
dest = reinterpret_cast<typeof( dest )>( resolve( symbolName ) )

#define LOAD_FREI0R_SYMBOL_CHECKED( dest, symbolName )  \
if ( ( LOAD_FREI0R_SYMBOL( dest, symbolName ) ) == NULL ) \
{                                                                       \
    qCritical() << "Failed to load symbol:" << symbolName;              \
    return false;                                                       \
}

bool
Effect::load()
{
    if ( isLoaded() == true )
        return true;
    LOAD_FREI0R_SYMBOL_CHECKED( m_f0r_init, "f0r_init" )
    LOAD_FREI0R_SYMBOL_CHECKED( m_f0r_deinit, "f0r_deinit" )
    LOAD_FREI0R_SYMBOL_CHECKED( m_f0r_info, "f0r_get_plugin_info" )
    LOAD_FREI0R_SYMBOL_CHECKED( m_f0r_construct, "f0r_construct" )
    LOAD_FREI0R_SYMBOL_CHECKED( m_f0r_destruct, "f0r_destruct" )
    LOAD_FREI0R_SYMBOL( m_f0r_update, "f0r_update" );
    LOAD_FREI0R_SYMBOL( m_f0r_update2, "f0r_update2" );
    LOAD_FREI0R_SYMBOL_CHECKED( m_f0r_get_param_value, "f0r_get_param_value" );
    LOAD_FREI0R_SYMBOL_CHECKED( m_f0r_set_param_value, "f0r_set_param_value" );
    LOAD_FREI0R_SYMBOL_CHECKED( m_f0r_get_param_info, "f0r_get_param_info" );

    //Initializing structures
    f0r_plugin_info_t   infos;

    m_f0r_init();
    m_f0r_info( &infos );
    m_name = infos.name;
    m_desc = infos.explanation;
    m_type = static_cast<Type>( infos.plugin_type );
    m_major = infos.major_version;
    m_minor = infos.minor_version;
    m_nbParams = infos.num_params;
    if ( m_type == Filter && m_f0r_update == NULL )
    {
        qCritical() << "Failed to load symbol f0r_update. Dropping module" << fileName();
        return false;
    }
    if ( ( m_type == Mixer2 || m_type == Mixer3 ) && m_f0r_update2 == NULL )
    {
        qCritical() << "Failed to load symbol f0r_update2. Dropping module" << fileName();
        return false;
    }
    for ( qint32 i = 0; i < m_nbParams; ++i )
    {
        f0r_param_info_t    *param = new f0r_param_info_t;
        m_f0r_get_param_info( param, i );
    }
    return true;
}

#undef LOAD_FREI0R_SYMBOL

const QString&
Effect::name()
{
    if ( isLoaded() == false )
        load();
    return m_name;
}

const QString&
Effect::description()
{
    if ( isLoaded() == false )
        load();
    return m_desc;
}

Effect::Type
Effect::type()
{
    if ( isLoaded() == false )
        load();
    return m_type;
}

int
Effect::getMajor()
{
    if ( m_major == -1 )
        load();
    return m_major;
}

int
Effect::getMinor()
{
    if ( m_minor == -1 )
        load();
    return m_minor;
}

EffectInstance*
Effect::createInstance()
{
    m_instCount.fetchAndAddAcquire( 1 );
    switch ( m_type )
    {
    case Filter:
        return new FilterInstance( this );
    default:
        return NULL;
    }
}

void
Effect::destroyInstance( EffectInstance *instance )
{
    delete instance;
    //fetchAndAddAcquire returns the old value.
    if ( m_instCount.fetchAndAddAcquire( -1 ) == 1 )
        unload();
}

const Effect::ParamList&
Effect::params() const
{
    return m_params;
}
