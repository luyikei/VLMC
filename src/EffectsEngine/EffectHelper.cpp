/*****************************************************************************
 * EffectHelper: Contains informations about effects
 *****************************************************************************
 * Copyright (C) 2008-2016 VideoLAN
 *
 * Authors: Yikei Lu    <luyikei.qmltu@gmail.com>
 *          Hugo Beauzée-Luyssen <hugo@beauzee.fr>
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
#include "Main/Core.h"
#include "Project/Project.h"
#include "Workflow/MainWorkflow.h"
#include "Backend/MLT/MLTFilter.h"
#include "Backend/IService.h"
#include "Backend/IBackend.h"
#include <mlt++/MltProperties.h>

QVariant
conv( std::string str, SettingValue::Type type )
{
    if ( str.empty() == true )
        return QVariant( QVariant::Invalid );

    if ( type == SettingValue::Double )
      return QVariant( std::stod( str ) );
    else if ( type == SettingValue::Int )
      return QVariant( std::stoi( str ) );
    else
      return QVariant( QString::fromStdString( str ) );
};

EffectHelper::EffectHelper( const char* id, qint64 begin, qint64 end,
                            const QString &uuid ) :
    Helper( uuid ),
    m_service( nullptr ),
    m_filterInfo( nullptr )
{
    try
    {
        m_filter = new Backend::MLT::MLTFilter( id );
    }
    catch ( Backend::InvalidServiceException& e )
    {
        m_filter = nullptr;
        throw e;
    }

    m_filter->setBoundaries( begin, end );
    initParams();
}

EffectHelper::EffectHelper( const QString& id, qint64 begin, qint64 end, const QString& uuid )
    : EffectHelper( qPrintable( id ), begin, end, uuid )
{

}

EffectHelper::EffectHelper( Backend::IFilter *filter, const QString& uuid )
    : Helper( uuid )
    , m_filter( dynamic_cast<Backend::MLT::MLTFilter*>( filter ) )
    , m_service( nullptr )
    , m_filterInfo( nullptr )
{
    if ( m_filter->isValid() == false )
        return;
    initParams();
}

EffectHelper::EffectHelper( const QVariant& variant )
    : EffectHelper( variant.toMap()["identifier"].toString() )
{
    loadFromVariant( variant );
}

EffectHelper::~EffectHelper()
{
    delete m_filter;
}

void
EffectHelper::initParams()
{
    for ( Backend::IParameterInfo* paramInfo : filterInfo()->paramInfos() )
    {
        SettingValue::Type type;

        // It treats as double internally
        if ( paramInfo->type() == "float" ||
             paramInfo->type() == "double" )
            type = SettingValue::Double;
        else if ( paramInfo->type() == "integer" )
            type = SettingValue::Int;
        else if ( paramInfo->type() == "boolean" )
            type = SettingValue::Bool;
        else
            type = SettingValue::String;

        SettingValue::Flags flags = SettingValue::Nothing;

        auto maxV = conv( paramInfo->maxValue(), type );
        auto minV = conv( paramInfo->maxValue(), type );
        if ( maxV != QVariant( QVariant::Invalid ) ||
             minV != QVariant( QVariant::Invalid ) )
            flags = SettingValue::Clamped;

        auto val = m_settings.createVar( type, QString::fromStdString( paramInfo->identifier() ),
                                         defaultValue( paramInfo->identifier().c_str(), type ), paramInfo->name().c_str(),
                                         paramInfo->description().c_str(), flags );

        connect( val, &SettingValue::changed, this, [this, val]( const QVariant& variant ) { set( val, variant ); } );
    }
}

Backend::IFilter*
EffectHelper::filter()
{
    return m_filter;
}

const Backend::IFilter*
EffectHelper::filter() const
{
    return m_filter;
}

void
EffectHelper::set( SettingValue* value, const QVariant& variant )
{
    auto key = value->key();

    switch ( value->type() )
    {
    case SettingValue::Double:
        m_filter->properties()->set( qPrintable( key ), variant.toDouble() );
        break;
    case SettingValue::Int:
        m_filter->properties()->set( qPrintable( key ), variant.toInt() );
        break;
    case SettingValue::Bool:
        m_filter->properties()->set( qPrintable( key ), variant.toBool() );
        break;
    default:
        m_filter->properties()->set( qPrintable( key ), qPrintable( variant.toString() ) );
        break;
    } ;
}

QVariant
EffectHelper::defaultValue( const char* id, SettingValue::Type type )
{
    switch ( type )
    {
    case SettingValue::Double:
        return QVariant( m_filter->properties()->get_double( id ) );
    case SettingValue::Int:
        return QVariant( m_filter->properties()->get_int( id ) );
    case SettingValue::Bool:
        return QVariant( (bool)m_filter->properties()->get_int( id ) );
    default:
        return QVariant( QString( m_filter->properties()->get( id ) ) );
    } ;
}

SettingValue*
EffectHelper::value( const QString& key )
{
    return m_settings.value( key );
}

void
EffectHelper::loadFromVariant( const QVariant& variant )
{
    auto m = variant.toMap()["parameters"].toMap();
    for ( auto it = m.cbegin(); it != m.cend(); ++it )
        value( it.key() )->set( it.value() );
}

QVariant
EffectHelper::toVariant()
{
    QVariantHash h;
    for ( const auto param : filterInfo()->paramInfos() )
    {
        auto val = value( QString::fromStdString( param->identifier() ) );
        h.insert( val->key(), val->get() );
    }
    return QVariantHash{ { "identifier", identifier() }, { "parameters", h } };
}

QVariant
EffectHelper::toVariant( Backend::IService* service )
{
    QVariantList filters;
    for ( int i = 0; i < service->filterCount(); ++ i )
    {
        EffectHelper helper( service->filter( i ) );
        filters << helper.toVariant();
    }
    return filters;
}

void
EffectHelper::loadFromVariant( const QVariant& variant, Backend::IService* service )
{
    for ( auto& var : variant.toList() )
    {
        EffectHelper helper( var );
        service->attach( *helper.filter() );
        helper.filter()->connect( *helper.filter() );
    }
}

qint64
EffectHelper::begin() const
{
    return m_filter->begin();
}

qint64
EffectHelper::end() const
{
    return m_filter->end();
}

void
EffectHelper::setBegin( qint64 begin )
{
    m_filter->setBoundaries( begin, end() );
}

void
EffectHelper::setEnd( qint64 end )
{
    m_filter->setBoundaries( begin(), end );
}

qint64
EffectHelper::length() const
{
    return m_filter->length();
}

void
EffectHelper::setBoundaries( qint64 begin, qint64 end )
{
    m_filter->setBoundaries( begin, end );
}

bool
EffectHelper::isValid() const
{
    return m_filter != nullptr && m_filter->isValid();
}

void
EffectHelper::setTarget( Backend::IService* service )
{
    if ( m_service )
        m_service->detach( *m_filter );
    m_service = service;
    m_service->attach( *m_filter );
    m_filter->connect( *m_service );
}

Backend::IService*
EffectHelper::target()
{
    return m_service;
}

Backend::IFilterInfo*
EffectHelper::filterInfo()
{
    if ( m_filterInfo == nullptr )
        m_filterInfo = Backend::instance()->filterInfo( m_filter->identifier() );
    return m_filterInfo;
}

QString
EffectHelper::identifier()
{
    return QString::fromStdString( m_filter->identifier() );
}
