/*****************************************************************************
 * TrackHandler.cpp : Handle multiple track of a kind (audio or video)
 *****************************************************************************
 * Copyright (C) 2008-2010 VideoLAN
 *
 * Authors: Hugo Beauzée-Luyssen <beauze.h@gmail.com>
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

#include "TrackHandler.h"
#include "TrackWorkflow.h"
#include "Workflow/Types.h"

#include <QDomDocument>
#include <QDomElement>

TrackHandler::TrackHandler( unsigned int nbTracks, Workflow::TrackType trackType ) :
        m_trackCount( nbTracks ),
        m_trackType( trackType ),
        m_length( 0 )
{
    m_tracks = new Toggleable<TrackWorkflow*>[nbTracks];
    for ( unsigned int i = 0; i < nbTracks; ++i )
    {
        m_tracks[i].setPtr( new TrackWorkflow( trackType ) );
        connect( m_tracks[i], SIGNAL( lengthChanged( qint64 ) ),
                 this, SLOT( lengthUpdated(qint64) ) );
    }
}

TrackHandler::~TrackHandler()
{
    for (unsigned int i = 0; i < m_trackCount; ++i)
        delete m_tracks[i];
    delete[] m_tracks;
}

void
TrackHandler::addClip( ClipHelper* ch, unsigned int trackId, qint64 start )
{
    Q_ASSERT_X( trackId < m_trackCount, "MainWorkflow::addClip",
                "The specified trackId isn't valid, for it's higher than the number of tracks");

    m_tracks[trackId]->addClip( ch, start );

    //Now check if this clip addition has changed something about the workflow's length
    if ( m_tracks[trackId]->getLength() > m_length )
        m_length = m_tracks[trackId]->getLength();
}

EffectsEngine::EffectHelper*
TrackHandler::addEffect( Effect *effect, quint32 trackId, const QUuid &uuid )
{
    return m_tracks[trackId]->addEffect( effect, uuid );
}

void
TrackHandler::startRender( quint32 width, quint32 height, double fps )
{
    m_endReached = false;
    computeLength();
    if ( m_length == 0 )
        m_endReached = true;
    else
    {
        for ( unsigned int i = 0; i < m_trackCount; ++i )
        {
            m_tracks[i]->initRender( width, height, fps );
        }
    }
}

void
TrackHandler::computeLength()
{
    qint64      maxLength = 0;

    for ( unsigned int i = 0; i < m_trackCount; ++i )
    {
        if ( m_tracks[i]->getLength() > maxLength )
            maxLength = m_tracks[i]->getLength();
    }
    m_length = maxLength;
}

qint64
TrackHandler::getLength() const
{
    return m_length;
}

Workflow::OutputBuffer*
TrackHandler::getOutput( qint64 currentFrame, qint64 subFrame, bool paused )
{
    bool        validTrack = false;

    for ( int i = m_trackCount - 1; i >= 0; --i )
    {
        if ( m_tracks[i].activated() == false || m_tracks[i]->hasNoMoreFrameToRender( currentFrame ) )
            continue ;
        validTrack = true;
        Workflow::OutputBuffer  *ret = m_tracks[i]->getOutput( currentFrame, subFrame, paused );
        if ( ret == NULL )
            continue ;
        else
            return ret;
    }
    if ( validTrack == false )
        allTracksEnded();
    return NULL;
}

qint64
TrackHandler::getClipPosition( const QUuid &uuid, unsigned int trackId ) const
{
    Q_ASSERT( trackId < m_trackCount );

    return m_tracks[trackId]->getClipPosition( uuid );
}

void
TrackHandler::stop()
{
    for (unsigned int i = 0; i < m_trackCount; ++i)
        m_tracks[i]->stop();
}

void
TrackHandler::moveClip(const QUuid &clipUuid, unsigned int oldTrack,
                                       unsigned int newTrack, qint64 startingFrame )
{
     Q_ASSERT( newTrack < m_trackCount && oldTrack < m_trackCount );

    if ( oldTrack == newTrack )
    {
        //And now, just move the clip.
        m_tracks[newTrack]->moveClip( clipUuid, startingFrame );
    }
    else
    {
        ClipWorkflow* cw = m_tracks[oldTrack]->removeClipWorkflow( clipUuid );
        m_tracks[newTrack]->addClip( cw, startingFrame );
    }
    computeLength();
}

Clip*
TrackHandler::removeClip( const QUuid& uuid, unsigned int trackId )
{
    Q_ASSERT( trackId < m_trackCount );

    Clip* clip = m_tracks[trackId]->removeClip( uuid );
    computeLength();
    return clip;
}

void
TrackHandler::muteTrack( unsigned int trackId )
{
    m_tracks[trackId].deactivate();
}

void
TrackHandler::unmuteTrack( unsigned int trackId )
{
    m_tracks[trackId].activate();
}

ClipHelper*
TrackHandler::getClipHelper( const QUuid& uuid, unsigned int trackId )
{
    Q_ASSERT( trackId < m_trackCount );

    return m_tracks[trackId]->getClipHelper( uuid );
}

void
TrackHandler::clear()
{
    for ( unsigned int i = 0; i < m_trackCount; ++i )
    {
        m_tracks[i]->clear();
    }
    m_length = 0;
}

bool
TrackHandler::endIsReached() const
{
    return m_endReached;
}

void
TrackHandler::allTracksEnded()
{
    m_endReached = true;
    emit tracksEndReached();
}

unsigned int
TrackHandler::getTrackCount() const
{
    return m_trackCount;
}

void
TrackHandler::save( QXmlStreamWriter& project ) const
{
    for ( unsigned int i = 0; i < m_trackCount; ++i)
    {
        if ( m_tracks[i]->getLength() > 0 )
        {
            project.writeStartElement( "track" );
            project.writeAttribute( "type", QString::number( (int)m_trackType ) );
            project.writeAttribute( "id", QString::number( i ) );
            m_tracks[i]->save( project );
            project.writeEndElement();
        }
    }
}

void
TrackHandler::renderOneFrame()
{
    for ( unsigned int i = 0; i < m_trackCount; ++i)
    {
        if ( m_tracks[i].activated() == true )
            m_tracks[i]->renderOneFrame();
    }
}

void
TrackHandler::setFullSpeedRender( bool val )
{
    for ( unsigned int i = 0; i < m_trackCount; ++i)
        m_tracks[i]->setFullSpeedRender( val );
}

void
TrackHandler::muteClip( const QUuid &uuid, quint32 trackId )
{
    m_tracks[trackId]->muteClip( uuid );
}

void
TrackHandler::unmuteClip( const QUuid &uuid, quint32 trackId )
{
    m_tracks[trackId]->unmuteClip( uuid );
}

bool
TrackHandler::contains( const QUuid &uuid ) const
{
    for ( unsigned int i = 0; i < m_trackCount; ++i )
        if ( m_tracks[i]->contains( uuid ) == true )
            return true;
    return false;
}

void
TrackHandler::stopFrameComputing()
{
    for ( unsigned int i = 0; i < m_trackCount; ++i )
        m_tracks[i]->stopFrameComputing();
}

void
TrackHandler::lengthUpdated( qint64 newLength )
{
    //If the new length is bigger, or if the track that has been resized was the
    if ( newLength > m_length )
        m_length = newLength;
    else
    {
        qint64      maxLength = 0;

        for ( unsigned int i = 0; i < m_trackCount; ++i )
        {
            if ( m_tracks[i]->getLength() > maxLength )
                maxLength = m_tracks[i]->getLength();
        }
        m_length = maxLength;
    }
}
