/*****************************************************************************
 * ConsoleRenderer.h: Handle the "server" mode rendering
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

#ifndef CONSOLERENDERER_H
#define CONSOLERENDERER_H

#include <QObject>
#include <QString>

class ConsoleRenderer : public QObject
{
    Q_OBJECT

public:
    explicit ConsoleRenderer( const QString& outputFileName, QObject *parent = 0 );

    void        startRender();

private:
    void        frameChanged( qint64 frame, qint64 length ) const;

private:
    QString                 m_outputFileName;

signals:
    void        finished();
};

#endif // CONSOLERENDERER_H
