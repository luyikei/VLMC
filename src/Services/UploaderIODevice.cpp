/*****************************************************************************
 * UploaderIODevice.cpp: Custom IODevice for uploading mutipart data and video
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
 * GNU General Public License for more dem_tails.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#include "UploaderIODevice.h"

UploaderIODevice::UploaderIODevice( QObject *parent, const QString& fileName,
                                    const QByteArray& head, const QByteArray& tail )
                                    : QIODevice( parent )
{
    m_file     = new QFile( fileName, this );
    m_head     = new QByteArray( head );
    m_tail     = new QByteArray( tail );
    m_position = 0;
}

UploaderIODevice::~UploaderIODevice()
{
    delete m_head;
    delete m_tail;
}

void
UploaderIODevice::setFile( const QString& fileName )
{
    if( m_file )
        delete m_file;

    m_file = new QFile( fileName, this );
}


/* Implement vitual method */
qint64 UploaderIODevice::readData( char *data, qint64 maxlen )
{
    if ( !m_file->isOpen() )
        return -1;

    char *pointer = data;
    qint64 atAll = 0;

    if ( ( m_position < m_head->size() ) && ( maxlen > 0 ) )
    {
        qint64 count = qMin( maxlen, ( qint64 ) m_head->size() );
        memcpy( pointer, m_head->data(), count );

        pointer    += count;
        m_position +=count;
        atAll      +=count;
        maxlen     -= count;
    }

    if ( ( maxlen > 0 ) && ( m_position < sizefull() ) )
    {
        qint64 count = qMin( maxlen, m_file->bytesAvailable() );
        int s = m_file->read( pointer, count );

        pointer    += s;
        maxlen     -= s;
        m_position += s;
        atAll      += s;
    }

    if ( m_position >= sizepart() && ( maxlen > 0 ) && ( m_position < sizefull() ) )
    {
        qint64 count = qMin( maxlen, ( qint64 ) m_tail->size() );
        memcpy( pointer, m_tail->data(), count );
        m_position += count;
        atAll      += count;
    }

    return atAll;
}

qint64 UploaderIODevice::writeData( const char *, qint64 )
{
    return -1;
}

qint64 UploaderIODevice::size() const
{
    return sizefull();
}

bool UploaderIODevice::openFile()
{
    if ( m_file->open( QIODevice::ReadOnly ) )
        return this->open( QIODevice::ReadOnly );
    return false;
}

qint64 UploaderIODevice::sizefull() const
{
    return ( m_file->size() + m_head->size() + m_tail->size() );
}

qint64 UploaderIODevice::sizepart() const
{
    return ( m_head->size() + m_file->size() );
}
