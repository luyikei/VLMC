/*****************************************************************************
 * Transcoder.cpp: Handle file transcoding.
 *****************************************************************************
 * Copyright (C) 2008-2014 VideoLAN
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

#include "Transcoder.h"

#include "Backend/ISource.h"
#include "Backend/ISourceRenderer.h"
#include "Media/Media.h"
#include "Metadata/MetaDataManager.h"
#include "NotificationZone.h"
#include "Settings/SettingsManager.h"
#include "Tools/RendererEventWatcher.h"

#include <QFileInfo>

Transcoder::Transcoder( Media* media )
    : m_media( media )
    , m_renderer( NULL )
{
    connect( this, SIGNAL( notify( QString ) ),
             NotificationZone::getInstance(), SLOT( notify( QString ) ) );
    connect( this, SIGNAL( progress( float ) ),
             NotificationZone::getInstance(), SLOT( progressUpdated( float ) ) );
    m_eventWatcher = new RendererEventWatcher;
}

Transcoder::~Transcoder()
{
    delete m_renderer;
    delete m_eventWatcher;
}

void
Transcoder::transcodeToPs()
{
    QString             outputDir = VLMC_PROJECT_GET_STRING( "vlmc/Workspace" );
    Backend::ISource*   source = m_media->source();
    delete m_renderer;
    m_renderer = source->createRenderer( m_eventWatcher );

    if ( outputDir.length() == 0 )
        outputDir = m_media->fileInfo()->absolutePath();
    m_destinationFile = outputDir + '/' + m_media->fileInfo()->baseName() + ".ps";
    m_renderer->setOutputFile( qPrintable( m_destinationFile ) );
    m_renderer->setName( qPrintable( QString( "Transcoder " ) + m_media->fileInfo()->baseName() ) );
    connect( m_eventWatcher, SIGNAL( positionChanged( float ) ), this, SIGNAL( progress( float ) ) );
    connect( m_eventWatcher, SIGNAL( endReached() ), this, SLOT( transcodeFinished() ) );
    emit notify( "Transcoding " + m_media->fileInfo()->absoluteFilePath() + " to " + m_destinationFile );
    m_renderer->start();
}

void
Transcoder::transcodeFinished()
{
    m_media->setFilePath( m_destinationFile );
    MetaDataManager::getInstance()->computeMediaMetadata( m_media );
    emit done();
    emit notify( m_media->fileInfo()->fileName() + ": Transcode finished" );
}
