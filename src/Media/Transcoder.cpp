/*****************************************************************************
 * Transcoder.cpp: Handle file transcoding.
 *****************************************************************************
 * Copyright (C) 2008-2016 VideoLAN
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <QFileInfo>

#include "Transcoder.h"

#include "Backend/VLC/VLCSource.h"
#include "Backend/VLC/VLCSourceRenderer.h"
#include "Media/Media.h"
#include "Metadata/MetaDataManager.h"
#include "Gui/widgets/NotificationZone.h"
#include "Settings/Settings.h"

Transcoder::Transcoder( Media* media )
    : m_media( media )
    , m_renderer( nullptr )
{
    connect( this, &Transcoder::notify,
             NotificationZone::instance(), &NotificationZone::notify );
    connect( this, &Transcoder::progress,
             NotificationZone::instance(), &NotificationZone::progressUpdated );
    m_eventWatcher = new Backend::VLC::RendererEventWatcher;
}

Transcoder::~Transcoder()
{
    delete m_renderer;
    delete m_eventWatcher;
}

void
Transcoder::transcodeToPs()
{
    QString             outputDir = VLMC_GET_STRING( "vlmc/Workspace" );
    auto*   source = m_media->source();
    delete m_renderer;
    m_renderer = source->createRenderer( m_eventWatcher );

    if ( outputDir.length() == 0 )
        outputDir = m_media->fileInfo()->absolutePath();
    m_destinationFile = outputDir + '/' + m_media->fileInfo()->baseName() + ".ps";
    m_renderer->setOutputFile( qPrintable( m_destinationFile ) );
    m_renderer->setName( qPrintable( QString( "Transcoder " ) + m_media->fileInfo()->baseName() ) );
    connect( m_eventWatcher, &Backend::VLC::RendererEventWatcher::positionChanged, [this](float pos) {
        emit progress(static_cast<int>( pos * 100 ) );
    });
    connect( m_eventWatcher, &Backend::VLC::RendererEventWatcher::endReached, this, &Transcoder::transcodeFinished );
    emit notify( "Transcoding " + m_media->fileInfo()->absoluteFilePath() + " to " + m_destinationFile );
    m_renderer->start();
}

void
Transcoder::transcodeFinished()
{
    m_media->setFilePath( m_destinationFile );
    MetaDataManager::instance()->computeMediaMetadata( m_media );
    emit done();
    emit notify( m_media->fileInfo()->fileName() + ": Transcode finished" );
}
