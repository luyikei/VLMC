/*****************************************************************************
 * MetaDataManager.cpp: Manages a thread to parse metadata
 *****************************************************************************
 * Copyright (C) 2008-2016 VideoLAN
 *
 * Authors: Christophe Courtaut <christophe.courtaut@gmail.com>
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

#include <QMutexLocker>

#include "Backend/ISource.h"
#include "Media/Media.h"
#include "MetaDataManager.h"
#include "Backend/VLC/VLCSource.h"

MetaDataManager::MetaDataManager()
    : m_computeInProgress( false )
{
}

MetaDataManager::~MetaDataManager()
{
}

void
MetaDataManager::computeMediaMetadata( Media *media )
{
    QMutexLocker lock( &m_computingMutex );
    m_mediaToCompute.enqueue( media );
    if ( m_computeInProgress == false )
        start();
}

void
MetaDataManager::run()
{
    m_computeInProgress = true;
    while ( true )
    {
        Media*  target;
        {
            QMutexLocker    lock( &m_computingMutex );
            if ( m_mediaToCompute.isEmpty() == true )
            {
                m_computeInProgress = false;
                return;
            }
            target = m_mediaToCompute.dequeue();
        }
        auto* targetSource = target->source();
        if ( targetSource->preparse() == false )
            emit failedToCompute( target );
        else
            target->onMetaDataComputed();
    }
}
