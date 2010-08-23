/*****************************************************************************
 * UploaderIODevice.h: Custom IODevice for uploading mutipart data and video
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

#ifndef UPLOADERIODEVICE_H
#define UPLOADERIODEVICE_H

#include <QFile>
#include <QIODevice>

class UploaderIODevice : public QIODevice
{
    Q_OBJECT

public:
    UploaderIODevice( QObject *parent, const QString& fileName,
                      const QByteArray& head, const QByteArray& tail );
    ~UploaderIODevice();

    void            setFile( const QString& fileName );
    bool            openFile();
    qint64          readData( char *data, qint64 maxlen );
    qint64          writeData( const char *data, qint64 len );
    qint64          size() const;

private:
    QFile*          m_file;
    QByteArray*     m_head;
    QByteArray*     m_tail;
    qint64          m_position;
    qint64          sizepart() const;
    qint64          sizefull() const;
};

#endif // UPLOADERIODEVICE_H
