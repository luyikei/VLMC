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
    cleanEffects();
    delete m_effectsLock;
}

EffectHelper*
EffectUser::addEffect( Effect *effect, qint64 start /*= 0*/, qint64 end /*= -1*/ )
{
    //Check that effect type is one of the supported ones
    if ( effect->type() == Effect::Filter  || effect->type() == Effect::Mixer2 )
    {
        EffectInstance  *effectInstance = effect->createInstance();
        EffectHelper *ret = new EffectHelper( effectInstance, start, end );
        addEffect( ret );
        return ret;
    }
    return nullptr;
}

void
EffectUser::addEffect( EffectHelper *effectHelper )
{
    if ( m_isRendering == true )
        effectHelper->effectInstance()->init( m_width, m_height );
    QWriteLocker    lock( m_effectsLock );
    if ( effectHelper->effectInstance()->effect()->type() == Effect::Filter )
        m_filters.push_back( effectHelper );
    else
        m_mixers.push_back( effectHelper );
    effectHelper->setTarget( this );
    emit effectAdded( effectHelper, effectHelper->begin() );
}

quint32*
EffectUser::applyFilters( const Workflow::Frame* frame, qint64 currentFrame )
{
    QReadLocker     lock( m_effectsLock );

    if ( m_filters.size() == 0 )
        return nullptr;
    EffectsEngine::EffectList::const_iterator     it = m_filters.constBegin();
    EffectsEngine::EffectList::const_iterator     ite = m_filters.constEnd();

    quint32         *buff1 = nullptr;
    quint32         *buff2 = nullptr;
    const quint32   *input = frame->buffer();
    bool            firstBuff = true;

    while ( it != ite )
    {
        if ( (*it)->begin() < currentFrame &&
             ( (*it)->end() < 0 || (*it)->end() > currentFrame ) )
        {
            quint32     **buff;
            if ( firstBuff == true )
                buff = &buff1;
            else
                buff = &buff2;
            if ( *buff == nullptr )
                *buff = new quint32[m_width * m_height];
            EffectInstance      *effect = (*it)->effectInstance();
            effect->process( input, *buff );
            input = *buff;
            firstBuff = !firstBuff;
        }
        ++it;
    }
    if ( buff1 != nullptr || buff2 != nullptr )
    {
        if ( firstBuff == true )
        {
            delete[] buff1;
            return buff2;
        }
        else
        {
            delete[] buff2;
            return buff1;
        }
    }
    return nullptr;
}

void
EffectUser::initFilters()
{
    QReadLocker     lock( m_effectsLock );

    EffectsEngine::EffectList::const_iterator   it = m_filters.begin();
    EffectsEngine::EffectList::const_iterator   ite = m_filters.end();

    while ( it != ite )
    {
        (*it)->effectInstance()->init( m_width, m_height );
        ++it;
    }
}

void
EffectUser::initMixers()
{
    QReadLocker     lock( m_effectsLock );

    EffectsEngine::EffectList::const_iterator   it = m_mixers.begin();
    EffectsEngine::EffectList::const_iterator   ite = m_mixers.end();

    while ( it != ite )
    {
        (*it)->effectInstance()->init( m_width, m_height );
        ++it;
    }
}

EffectHelper*
EffectUser::getMixer( qint64 currentFrame )
{
    QReadLocker     lock( m_effectsLock );

    EffectsEngine::EffectList::const_iterator      it = m_mixers.constBegin();
    EffectsEngine::EffectList::const_iterator      ite = m_mixers.constEnd();

    while ( it != ite )
    {
        if ( (*it)->begin() <= currentFrame && currentFrame <= (*it)->end() )
        {
            Q_ASSERT( (*it)->effectInstance()->effect()->type() == Effect::Mixer2 );
            return (*it);
        }
        ++it;
    }
    return nullptr;
}

QVariant
EffectUser::toVariant() const
{
    QVariantList l;
    for ( const auto& filter : m_filters )
    {
        l << QVariantHash{
                { "name", filter->effectInstance()->effect()->name() },
                { "start", filter->begin() },
                { "end", filter->end() }
            };
    }
    return QVariant( l );
}

void
EffectUser::loadFromVariant( const QVariant& variant )
{
    for ( const auto& var : variant.toList() )
    {
        QVariantMap m = var.toMap();
        const QString& name = m["name"].toString();
        qint64 start = m["start"].toLongLong();
        qint64 end = m["end"].toLongLong();
        if ( name.isEmpty() == false )
        {
            Effect  *e = Core::instance()->effectsEngine()->effect( name );
            if ( e != nullptr )
            {
                EffectHelper    *helper = addEffect( e, start, end );
                if ( helper == nullptr )
                    vlmcCritical() << "Can't load effect" << name;
            }
            else
                vlmcCritical() << "Can't load effect" << name;
        }
    }
}

const EffectsEngine::EffectList&
EffectUser::effects( Effect::Type type ) const
{
    if ( type == Effect::Filter )
        return m_filters;
    if ( type != Effect::Mixer2 )
        vlmcCritical() << "Only Filters and Mixer2 are handled. This is going to be nasty !";
    return m_mixers;
}

void
EffectUser::removeEffect( Effect::Type type, qint32 idx )
{
    Q_ASSERT( idx >= 0 );

    QWriteLocker    lock( m_effectsLock );

    if ( type == Effect::Filter )
    {
        if ( idx < m_filters.size() )
        {
            EffectHelper    *helper = m_filters.takeAt( idx );
            helper->setTarget( nullptr );
            emit    effectRemoved( helper->uuid() );
        }
    }
    else if ( type == Effect::Mixer2 )
    {
        if ( idx < m_mixers.size() )
        {
            EffectHelper    *helper = m_mixers.takeAt( idx );
            helper->setTarget( nullptr );
            emit    effectRemoved( helper->uuid() );
        }
    }
    else
        vlmcCritical() << "Unhandled effect type";
}

void
EffectUser::removeEffect(EffectHelper *helper)
{
    QWriteLocker    lock( m_effectsLock );

    EffectsEngine::EffectList::iterator     it = m_filters.begin();
    EffectsEngine::EffectList::iterator     ite = m_filters.end();
    while ( it != ite )
    {
        if ( (*it)->uuid() == helper->uuid() )
        {
            EffectHelper    *eh = *it;
            eh->setTarget( nullptr );
            m_filters.erase( it );
            emit effectRemoved( eh->uuid() );
            return ;
        }
        ++it;
    }
    vlmcWarning() << "Can't find EffectHelper" << helper->uuid() << "for removal.";
}

void
EffectUser::swapFilters( qint32 idx, qint32 idx2 )
{
    Q_ASSERT( idx >= 0 && idx2 >= 0 );

    if ( idx >= m_filters.size() || idx2 >= m_filters.size() )
        return ;
    QWriteLocker        lock( m_effectsLock );

    m_filters.swap( idx, idx2 );
}

qint32
EffectUser::count( Effect::Type type ) const
{
    QReadLocker     lock( m_effectsLock );

    if ( type == Effect::Filter )
        return m_filters.count();
    if ( type == Effect::Mixer2 )
        return m_mixers.count();
    vlmcCritical() << "Unhandled effect type";
    return 0;
}

void
EffectUser::moveEffect( EffectHelper *helper, qint64 newPos )
{
    QWriteLocker    lock( m_effectsLock );

    foreach ( EffectHelper *eh, m_filters )
    {
        if ( helper->uuid() == eh->uuid() )
        {
            qint64  offset = helper->begin() - newPos;
            helper->setBoundaries( newPos, helper->end() - offset );
            emit effectMoved( helper, newPos );
            return ;
        }
    }
    vlmcWarning() << this << "Can't find effect" << helper->uuid();
}

void
EffectUser::cleanEffects()
{
    QWriteLocker    lock( m_effectsLock );

    qDeleteAll( m_filters );
    qDeleteAll( m_mixers );
    m_filters.clear();
    m_mixers.clear();
}

bool
EffectUser::contains( Effect::Type, const QUuid &uuid ) const
{
    QReadLocker     lock( m_effectsLock );

    EffectsEngine::EffectList::const_iterator     it = m_filters.constBegin();
    EffectsEngine::EffectList::const_iterator     ite = m_filters.constEnd();
    while ( it != ite )
    {
        if ( (*it)->uuid() == uuid )
            return true;
        ++it;
    }
    return false;
}
