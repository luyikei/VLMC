/*****************************************************************************
 * GUIMedia.cpp: Represents the GUI part of a Media
 *****************************************************************************
 * Copyright (C) 2008-2010 VideoLAN
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

#include "GuiMedia.h"
#include "Media.h"

QPixmap*        GUIMedia::defaultSnapshot = NULL;

GUIMedia::GUIMedia() :
    m_snapshot( NULL )
{
}

void GUIMedia::snapshotReady(const char *fileName)
{
    QFile   tmp( fileName );

    QPixmap* pixmap = new QPixmap( fileName );
    if ( pixmap->isNull() )
        delete pixmap;
    else
    {
        setSnapshot( pixmap );
        emit snapshotComputed( qobject_cast<const Media*>( this ) );
    }
    tmp.remove();
}

GUIMedia::~GUIMedia()
{
    if ( m_snapshot )
        delete m_snapshot;
}

void
GUIMedia::setSnapshot( QPixmap* snapshot )
{
    if ( m_snapshot != NULL )
        delete m_snapshot;
    m_snapshot = snapshot;
}

const QPixmap&
GUIMedia::snapshot() const
{
    if ( m_snapshot != NULL )
        return *m_snapshot;
    if ( GUIMedia::defaultSnapshot == NULL )
        GUIMedia::defaultSnapshot = new QPixmap( ":/images/vlmc" );
    return *GUIMedia::defaultSnapshot;
}

bool
GUIMedia::hasSnapshot() const
{
    return ( m_snapshot != NULL );
}
