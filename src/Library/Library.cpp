/*****************************************************************************
 * Library.cpp: Multimedia library
 *****************************************************************************
 * Copyright (C) 2008-2010 VideoLAN
 *
 * Authors: Hugo Beauzee-Luyssen <hugo@vlmc.org>
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

/** \file
  * This file the library contains class implementation.
  * It's the the backend part of the Library widget of vlmc.
  * It can load and unload Medias (Medias.h/Media.cpp)
  * It can load and unload Clips (Clip.h/Clip.cpp)
  */

#include "Clip.h"
#include "Library.h"
#include "Media.h"
#include "MetaDataManager.h"

#include <QtDebug>
#include <QDomElement>
#include <QHash>
#include <QUuid>

void
Library::loadProject( const QDomElement& medias )
{
    if ( medias.isNull() == true || medias.tagName() != "medias" )
    {
        qWarning() << "Invalid medias node";
        return ;
    }

    QDomElement elem = medias.firstChild().toElement();
    while ( elem.isNull() == false )
    {
        QList<QDomElement>  clipList;
        QDomElement mediaProperty = elem.firstChild().toElement();
        QString     path;
        QString     uuid;

        while ( mediaProperty.isNull() == false )
        {
            QString tagName = mediaProperty.tagName();
            if ( tagName == "path" )
                path = mediaProperty.text();
            else if ( tagName == "uuid" )
                uuid = mediaProperty.text();
            else if ( tagName == "clips" )
            {
                QDomElement clip = mediaProperty.firstChild().toElement();
                while ( clip.isNull() == false )
                {
                    clipList.push_back( clip );
                    clip = clip.nextSibling().toElement();
                }
            }
            else
                qWarning() << "Unknown field" << tagName;
            mediaProperty = mediaProperty.nextSibling().toElement();
        }
        //FIXME: This is verry redondant...
        if ( mediaAlreadyLoaded( path ) == true )
        {
            QHash<QUuid, Clip*>::iterator   it = m_clips.begin();
            QHash<QUuid, Clip*>::iterator   end = m_clips.end();

            for ( ; it != end; ++it )
            {
                if ( it.value()->getMedia()->fileInfo()->absoluteFilePath() == path )
                {
                    Clip*   clip = it.value();
                    clip->setUuid( QUuid( uuid ) );
                    m_clips.erase( it );
                    m_clips[clip->uuid()] = clip;
                    break ;
                }
            }
        }
        else
        {
            if ( addMedia( path, uuid ) == false )
            {
                elem = elem.nextSibling().toElement();
                continue ;
            }
        }
        if ( clipList.size() != 0 )
        {
            foreach( QDomElement clip, clipList )
            {
                QString parentUuid = clip.attribute( "parentUuid", "");
                if ( parentUuid != "" && parentUuid == uuid )
                {
                    QString beg = clip.attribute( "begin", "" );
                    QString end = clip.attribute( "end", "" );
                    QString clipUuid = clip.attribute( "uuid", "" );
                    if ( beg != "" && end != "" && uuid != "" )
                    {
                        if ( m_clips.contains( uuid ) == false )
                            continue ;
                        Clip* parentClip = m_clips[QUuid( uuid )];
                        Clip* clip = new Clip( parentClip, beg.toInt(), end.toInt(), QUuid( clipUuid ) );
                        parentClip->addSubclip( clip );
                    }
                }
            }
        }
        elem = elem.nextSibling().toElement();
    }
    emit projectLoaded();
}

void
Library::saveProject( QDomDocument& doc, QDomElement& rootNode )
{
//    QHash<QUuid, Media*>::iterator          it = m_clips.begin();
//    QHash<QUuid, Media*>::iterator          end = m_clips.end();
//
//    QDomElement medias = doc.createElement( "medias" );
//
//    for ( ; it != end; ++it )
//    {
//        QDomElement media = doc.createElement( "media" );
//
//        medias.appendChild( media );
//        QDomElement mrl = doc.createElement( "path" );
//
//        QDomCharacterData text;
//        text = doc.createTextNode( it.value()->fileInfo()->absoluteFilePath() );
//
//        QDomElement uuid = doc.createElement( "uuid" );
//        QDomCharacterData text2 = doc.createTextNode( it.value()->baseClip()->uuid().toString() );
//
//        mrl.appendChild( text );
//        uuid.appendChild( text2 );
//
//        media.appendChild( mrl );
//        media.appendChild( uuid );
//        //Creating the clip branch
//        if ( it.value()->clipsCount() != 0 )
//        {
//            QDomElement clips = doc.createElement( "clips" );
//            foreach( Clip* c, it.value()->clips().values() )
//            {
//                QDomElement clip = doc.createElement( "clip" );
//                clip.setAttribute( "begin", c->begin() );
//                clip.setAttribute( "end", c->end() );
//                clip.setAttribute( "uuid", c->uuid() );
//                clip.setAttribute( "parentUuid", c->getMedia()->baseClip()->uuid() );
//                clips.appendChild( clip );
//            }
//            media.appendChild( clips );
//        }
//    }
//    rootNode.appendChild( medias );
    #warning "FIXME: Project saving";
}
