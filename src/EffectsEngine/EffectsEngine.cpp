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
#include "EffectInstance.h"
#include "Types.h"

#include <QDesktopServices>
#include <QDir>
#include <QSettings>
#include <QtDebug>

EffectsEngine::EffectsEngine()
{
    m_cache = new QSettings( QDesktopServices::storageLocation(
                    QDesktopServices::CacheLocation ) + "/effects",
                             QSettings::IniFormat, this );
}

EffectsEngine::~EffectsEngine()
{
}

EffectsEngine::EffectHelper::EffectHelper( EffectInstance *_effect, qint64 _start, qint64 _end,
                                           const QString& _uuid ) :
        effect( _effect ),
        start( _start ),
        end( _end )
{
    if ( _uuid.isNull() == true )
        uuid = QUuid::createUuid();
    else
        uuid = _uuid;
}

Effect*
EffectsEngine::effect( const QString& name )
{
    QHash<QString, Effect*>::iterator   it = m_effects.find( name );
    if ( it != m_effects.end() )
        return it.value();
    return NULL;
}

bool
EffectsEngine::loadEffect( const QString &fileName )
{
    Effect*         e = new Effect( fileName );
    QString         name;
    Effect::Type    type;

    if ( m_cache->contains( fileName + "/name" ) == true &&
         m_cache->contains( fileName + "/type" ) == true )
    {
        name = m_cache->value( fileName + "/name" ).toString();
        int     typeInt = m_cache->value( fileName + "/type" ).toInt();
        if ( typeInt < Effect::Unknown || typeInt > Effect::Mixer3 )
            qWarning() << "Invalid plugin type.";
        else
        {
            type = static_cast<Effect::Type>( typeInt );
            m_effects[name] = e;
            emit effectAdded( e, name, type );
            return true;
        }
    }
    if ( e->load() == false )
    {
        delete e;
        return false;
    }
    m_effects[e->name()] = e;
    m_cache->setValue( fileName + "/name", e->name() );
    m_cache->setValue( fileName + "/type", e->type() );
    name = e->name();
    type = e->type();
    emit effectAdded( e, name, type );
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

void
EffectsEngine::applyEffects( const EffectList &effects, Workflow::Frame* frame,
                             qint64 currentFrame  )
{
    if ( effects.size() == 0 )
        return ;
    EffectList::const_iterator     it = effects.constBegin();
    EffectList::const_iterator     ite = effects.constEnd();

    quint8      *buff1 = NULL;
    quint8      *buff2 = NULL;
    quint8      *input = frame->buffer();
    bool        firstBuff = true;

    while ( it != ite )
    {
        if ( (*it)->start < currentFrame &&
             ( (*it)->end < 0 || (*it)->end > currentFrame ) )
        {
            quint8      **buff;
            if ( firstBuff == true )
                buff = &buff1;
            else
                buff = &buff2;
            if ( *buff == NULL )
                *buff = new quint8[frame->size()];
            EffectInstance      *effect = (*it)->effect;
            effect->process( 0.0, (quint32*)input, (quint32*)*buff );
            input = *buff;
            firstBuff = !firstBuff;
        }
        ++it;
    }
    if ( buff1 != NULL || buff2 != NULL )
    {
        //The old input frame will automatically be deleted when setting the new buffer
        if ( firstBuff == true )
        {
            delete[] buff1;
            frame->setBuffer( buff2 );
        }
        else
        {
            delete[] buff2;
            frame->setBuffer( buff1 );
        }
    }
}

void
EffectsEngine::saveEffects( const EffectList &effects, QXmlStreamWriter &project )
{
    if ( effects.size() <= 0 )
        return ;
    EffectsEngine::EffectList::const_iterator   it = effects.begin();
    EffectsEngine::EffectList::const_iterator   ite = effects.end();
    project.writeStartElement( "effects" );
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

void
EffectsEngine::initEffects( const EffectList &effects, quint32 width, quint32 height )
{
    EffectsEngine::EffectList::const_iterator   it = effects.begin();
    EffectsEngine::EffectList::const_iterator   ite = effects.end();

    while ( it != ite )
    {
        (*it)->effect->init( width, height );
        ++it;
    }
}
