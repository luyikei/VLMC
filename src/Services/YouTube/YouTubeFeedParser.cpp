/*****************************************************************************
 * YouTubeFeedParser.cpp: YouTube XML feed parser
 *****************************************************************************
 * Copyright (C) 2010 VideoLAN
 *
 * Authors: Rohit Yadav <rohityadav89 AT gmail.com>
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

#include "YouTubeFeedParser.h"
#include <QString>
#include <QStringList>

#include <QDebug>

YouTubeFeedParser::YouTubeFeedParser( const QString& xml )
    : QXmlStreamReader( xml )
{
    m_videoId = "";
}

bool
YouTubeFeedParser::read()
{
    while( !atEnd() )
    {
        readNext();
        if( isStartElement() )
        {
            if( name() == "entry")
                readFeed();
            else
                raiseError( QObject::tr( "The XMLStream is not a valid YouTube Feed" ) );
        }
    }

    return !error();
}

void
YouTubeFeedParser::readUnknownElement()
{
    Q_ASSERT( isStartElement() );

    while( !atEnd() )
    {
        readNext();

        if( isEndElement() )
            break;

        if( isStartElement() )
            readUnknownElement();
    }
}

void
YouTubeFeedParser::readFeed()
{
    Q_ASSERT( isStartElement() && name() == "entry" );

    while( !atEnd() )
    {
        readNext();

        if( isEndElement() )
            break;

        if( isStartElement() )
        {
            if( name() == "link" )
                readLinks();
            else
                readUnknownElement();
        }
    }
}

void
YouTubeFeedParser::readLinks()
{
    Q_ASSERT( isStartElement() && name() == "link" );

    while( !atEnd() )
    {
        if( isEndElement() )
            break;

        if( isStartElement() && name() == "link" )
        {
            QXmlStreamAttributes attrs = attributes();
            QStringRef rel = attrs.value("rel");
            QStringRef type = attrs.value("type");
            QStringRef href = attrs.value("href");

            if( rel.toString() == "self" )
            {
                m_videoId = href.toString();
                m_videoId = m_videoId.split("uploads/").at(1);
            }
        }
        readNext();
    }
}

const QString&
YouTubeFeedParser::getVideoId()
{
    return m_videoId;
}
