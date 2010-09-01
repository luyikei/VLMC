/*****************************************************************************
 * EffectUser.cpp: Handles effects list and application
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

#include "EffectUser.h"

#include "EffectInstance.h"
#include "Types.h"

#include <QDomElement>
#include <QReadWriteLock>
#include <QtDebug>

EffectUser::EffectUser() :
        m_isRendering( false ),
        m_width( 0 ),
        m_height( 0 )
{
    m_effectsLock = new QReadWriteLock;
}

EffectUser::~EffectUser()
{
    delete m_effectsLock;
}

EffectsEngine::EffectHelper*
EffectUser::addEffect( Effect *effect, qint64 start /*= 0*/, qint64 end /*= -1*/ )
{
    //FIXME: Check it the effect type is supported
    EffectInstance  *effectInstance = effect->createInstance();
    if ( m_isRendering == true )
        effectInstance->init( m_width, m_height );
    EffectsEngine::EffectHelper *ret = new EffectsEngine::EffectHelper( effectInstance, start, end );

    QWriteLocker    lock( m_effectsLock );
    if ( effect->type() == Effect::Filter )
        m_filters.push_back( ret );
    else
        m_mixers.push_back( ret );
    return ret;
}


quint32*
EffectUser::applyFilters( const Workflow::Frame* frame,
                             qint64 currentFrame, double time )
{
    QReadLocker     lock( m_effectsLock );

    if ( m_filters.size() == 0 )
        return NULL;
    EffectsEngine::EffectList::const_iterator     it = m_filters.constBegin();
    EffectsEngine::EffectList::const_iterator     ite = m_filters.constEnd();

    quint32         *buff1 = NULL;
    quint32         *buff2 = NULL;
    const quint32   *input = frame->buffer();
    bool            firstBuff = true;

    while ( it != ite )
    {
        if ( (*it)->start < currentFrame &&
             ( (*it)->end < 0 || (*it)->end > currentFrame ) )
        {
            quint32     **buff;
            if ( firstBuff == true )
                buff = &buff1;
            else
                buff = &buff2;
            if ( *buff == NULL )
                *buff = new quint32[frame->nbPixels()];
            EffectInstance      *effect = (*it)->effect;
            effect->process( time, input, *buff );
            input = *buff;
            firstBuff = !firstBuff;
        }
        ++it;
    }
    if ( buff1 != NULL || buff2 != NULL )
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
    return NULL;
}

void
EffectUser::initFilters()
{
    QReadLocker     lock( m_effectsLock );

    EffectsEngine::EffectList::const_iterator   it = m_filters.begin();
    EffectsEngine::EffectList::const_iterator   ite = m_filters.end();

    while ( it != ite )
    {
        (*it)->effect->init( m_width, m_height );
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
        (*it)->effect->init( m_width, m_height );
        ++it;
    }
}

EffectsEngine::EffectHelper*
EffectUser::getMixer( qint64 currentFrame )
{
    QReadLocker     lock( m_effectsLock );

    EffectsEngine::EffectList::const_iterator      it = m_mixers.constBegin();
    EffectsEngine::EffectList::const_iterator      ite = m_mixers.constEnd();

    while ( it != ite )
    {
        if ( (*it)->start <= currentFrame && currentFrame <= (*it)->end )
        {
            Q_ASSERT( (*it)->effect->effect()->type() == Effect::Mixer2 );
            return (*it);
        }
        ++it;
    }
    return NULL;
}

void
EffectUser::loadEffects( const QDomElement &parent )
{
    QDomElement     effects = parent.firstChildElement( "effects" );
    if ( effects.isNull() == true )
        return ;
    QDomElement     effect = effects.firstChildElement( "effect" );
    while ( effect.isNull() == false )
    {
        if ( effect.hasAttribute( "name" ) == true &&
             effect.hasAttribute( "start" ) == true &&
             effect.hasAttribute( "end" ) == true )
        {
            Effect  *e = EffectsEngine::getInstance()->effect( effect.attribute( "name" ) );
            if ( e != NULL )
                addEffect( e, effect.attribute( "start" ).toLongLong(),
                              effect.attribute( "end" ).toLongLong() );
            else
                qCritical() << "Renderer: Can't load effect" << effect.attribute( "name" );
        }
        effect = effect.nextSiblingElement();
    }
}

void
EffectUser::saveFilters( QXmlStreamWriter &project ) const
{
    QReadLocker     lock( m_effectsLock );

    if ( m_filters.size() <= 0 )
        return ;
    project.writeStartElement( "effects" );
    EffectsEngine::EffectList::const_iterator   it = m_filters.begin();
    EffectsEngine::EffectList::const_iterator   ite = m_filters.end();
    while ( it != ite )
    {
        project.writeStartElement( "effect" );
        project.writeAttribute( "name", (*it)->effect->effect()->name() );
        project.writeAttribute( "start", QString::number( (*it)->start ) );
        project.writeAttribute( "end", QString::number( (*it)->end ) );
        project.writeEndElement();
        ++it;
    }
    project.writeEndElement();
}

