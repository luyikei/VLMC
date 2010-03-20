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

class   WorkflowFileRenderer;

#include <QObject>
#include <QString>

class ConsoleRenderer : public QObject
{
    Q_OBJECT

public:
    explicit ConsoleRenderer( QObject *parent = 0 );

public slots:
    void        startRender();

private:
    WorkflowFileRenderer    *m_renderer;
    QString                 m_outputFileName;
    quint32                 m_width;
    quint32                 m_height;
    double                  m_fps;
    quint32                 m_vbitrate;
    quint32                 m_abitrate;
};

#endif // CONSOLERENDERER_H
