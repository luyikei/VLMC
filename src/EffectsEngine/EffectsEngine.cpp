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
#include <QXmlStreamWriter>

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
            if ( m_effects.contains( name ) == true )
            {
                delete e;
                return false;
            }
            m_effects[name] = e;
            emit effectAdded( e, name, type );
            return true;
        }
    }
    if ( e->load() == false || m_effects.contains( e->name() ) == true )
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
    const QFileInfoList& files = dir.entryInfoList( QDir::Files | QDir::NoDotAndDotDot |
                                              QDir::Readable | QDir::Executable );
    foreach ( const QFileInfo& file, files )
    {
        if ( file.isDir() )
            browseDirectory( file.absoluteFilePath() );
        else
            loadEffect( file.absoluteFilePath() );
    }
}

quint32*
EffectsEngine::applyFilters( const EffectList &effects, const Workflow::Frame* frame,
                             qint64 currentFrame, double time )
{
    if ( effects.size() == 0 )
        return NULL;
    EffectList::const_iterator     it = effects.constBegin();
    EffectList::const_iterator     ite = effects.constEnd();

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
EffectsEngine::saveFilters( const EffectList &effects, QXmlStreamWriter &project )
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

EffectsEngine::EffectHelper*
EffectsEngine::getMixer( const EffectList &mixers, qint64 currentFrame )
{
    EffectList::const_iterator      it = mixers.constBegin();
    EffectList::const_iterator      ite = mixers.constEnd();

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
EffectsEngine::loadEffects()
{
    //FIXME: What should we do for windows ?!
    //Refer to http://www.piksel.org/frei0r/1.2/spec/group__pluglocations.html
    const   QString paths[3] = {
        QString( QDir::homePath() + "/.frei0r-1/lib/" ),
        QString("/usr/local/lib/frei0r-1/"),
        QString("/usr/lib/frei0r-1/" )
    };
    for ( quint32 i = 0; i < 3; ++i )
    {
        if ( QFile::exists( paths[i] ) == true )
        {
            browseDirectory( paths[i] );
        }
    }
}
