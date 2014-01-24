/*****************************************************************************
 * GUIMedia.h: Represents the GUI part of a Media
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

#ifndef GUIMEDIA_H
#define GUIMEDIA_H

#include <QObject>
#include <QPixmap>

class   QPixmap;
class   Media;

class GUIMedia : public QObject
{
    Q_OBJECT

public:
    ~GUIMedia();
    void                        setSnapshot( QPixmap* snapshot );
    const QPixmap               &snapshot() const;
    bool                        hasSnapshot() const;

protected:
    //A GUIMedia shouldn't be constructed by something else than a media
    GUIMedia();

    static QPixmap*             defaultSnapshot;
    QPixmap*                    m_snapshot;


public slots:
    void                        snapshotReady( const char *fileName );

signals:
    void                        snapshotComputed( const Media* );
};

#endif // GUIMEDIA_H
