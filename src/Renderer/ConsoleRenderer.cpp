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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "ConsoleRenderer.h"
#include "Main/Core.h"
#include "Project/Project.h"
#include "Tools/VlmcDebug.h"
#include "Workflow/MainWorkflow.h"

ConsoleRenderer::ConsoleRenderer( const QString& outputFileName, QObject *parent )
    : QObject( parent )
    , m_outputFileName( outputFileName )
{
    connect( Core::instance()->workflow(), &MainWorkflow::frameChanged,
             this, &ConsoleRenderer::frameChanged, Qt::DirectConnection );
}

void
ConsoleRenderer::frameChanged( qint64 frame, qint64 length ) const
{
    static qint64   percent = 0;
    qint64          newPercent;

    newPercent = ( frame + 1 ) * 100 / length; // The frame is 0-indexed
    if ( newPercent != percent )
    {
        percent = newPercent;
        vlmcDebug() << "ConsoleRenderer:" << percent << "%";
    }
}

void
ConsoleRenderer::startRender()
{
    auto project = Core::instance()->project();
    Core::instance()->workflow()->startRenderToFile( m_outputFileName,
                                                     project->width(),
                                                     project->height(),
                                                     project->fps(),
                                                     project->aspectRatio(),
                                                     project->videoBitrate(),
                                                     project->audioBitrate(),
                                                     project->nbChannels(),
                                                     project->sampleRate()
                                                     );
    emit finished();
}
