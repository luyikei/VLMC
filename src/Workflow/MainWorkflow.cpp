/*****************************************************************************
 * MainWorkflow.cpp : Will query all of the track workflows to render the final
 *                    image
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

#include "vlmc.h"
#include "Project/Project.h"
#include "Media/Clip.h"
#include "ClipHelper.h"
#include "ClipWorkflow.h"
#include "Library/Library.h"
#include "MainWorkflow.h"
#include "Project/Project.h"
#include "TrackWorkflow.h"
#include "TrackHandler.h"
#include "Settings/Settings.h"
#include "Tools/VlmcDebug.h"
#include "Workflow/Types.h"

#include <QDomElement>
#include <QMutex>

MainWorkflow::MainWorkflow( int trackCount ) :
        m_blackOutput( NULL ),
        m_lengthFrame( 0 ),
        m_renderStarted( false ),
        m_width( 0 ),
        m_height( 0 ),
        m_trackCount( trackCount )
{
    m_currentFrameLock = new QReadWriteLock;

    m_tracks = new TrackHandler*[Workflow::NbTrackType];
    m_currentFrame = new qint64[Workflow::NbTrackType];
    for ( unsigned int i = 0; i < Workflow::NbTrackType; ++i )
    {
        Workflow::TrackType trackType = static_cast<Workflow::TrackType>(i);
        m_tracks[i] = new TrackHandler( trackCount, trackType );
        connect( m_tracks[i], SIGNAL( tracksEndReached() ),
                 this, SLOT( tracksEndReached() ) );
        connect( m_tracks[i], SIGNAL( lengthChanged(qint64) ),
                 this, SLOT( lengthUpdated( qint64 ) ) );
        m_currentFrame[i] = 0;
    }
}

MainWorkflow::~MainWorkflow()
{
    delete m_currentFrameLock;
    delete m_currentFrame;
    for ( unsigned int i = 0; i < Workflow::NbTrackType; ++i )
        delete m_tracks[i];
    delete[] m_tracks;
    delete m_blackOutput;
}

void
MainWorkflow::computeLength()
{
    qint64      maxLength = 0;

    for ( unsigned int i = 0; i < Workflow::NbTrackType; ++i )
    {
        if ( m_tracks[i]->getLength() > maxLength )
            maxLength = m_tracks[i]->getLength();
    }
    if ( m_lengthFrame != maxLength )
    {
        m_lengthFrame = maxLength;
        emit lengthChanged( m_lengthFrame );
    }

}

void
MainWorkflow::startRender( quint32 width, quint32 height, double fps )
{
    //Reinit the effects in case the width/height has change
    m_renderStarted = true;
    m_width = width;
    m_height = height;
    if ( m_blackOutput != NULL )
        delete m_blackOutput;
    m_blackOutput = new Workflow::Frame( m_width, m_height );
    memset( m_blackOutput->buffer(), 0, m_blackOutput->size() );
    for ( unsigned int i = 0; i < Workflow::NbTrackType; ++i )
        m_tracks[i]->startRender( width, height, fps );
    computeLength();
}

const Workflow::OutputBuffer*
MainWorkflow::getOutput( Workflow::TrackType trackType, bool paused )
{
    if ( m_renderStarted == true )
    {
        qint64              currentFrame;
        qint64              subFrame;

        {
            QReadLocker         lock2( m_currentFrameLock );
            currentFrame = m_currentFrame[Workflow::VideoTrack];
            subFrame = m_currentFrame[trackType];
        }

        Workflow::OutputBuffer  *ret = m_tracks[trackType]->getOutput( currentFrame,
                                                                       subFrame, paused );
        if ( trackType == Workflow::VideoTrack )
        {
            if ( ret == NULL )
                return m_blackOutput;
        }
        return ret;
    }
    return NULL;
}

void
MainWorkflow::nextFrame( Workflow::TrackType trackType )
{
    QWriteLocker    lock( m_currentFrameLock );

    ++m_currentFrame[trackType];
    if ( trackType == Workflow::VideoTrack )
        emit frameChanged( m_currentFrame[Workflow::VideoTrack], Vlmc::Renderer );
}

void
MainWorkflow::previousFrame( Workflow::TrackType trackType )
{
    QWriteLocker    lock( m_currentFrameLock );

    --m_currentFrame[trackType];
    if ( trackType == Workflow::VideoTrack )
        emit frameChanged( m_currentFrame[Workflow::VideoTrack], Vlmc::Renderer );
}

qint64
MainWorkflow::getLengthFrame() const
{
    return m_lengthFrame;
}

qint64
MainWorkflow::getClipPosition( const QUuid& uuid, unsigned int trackId,
                               Workflow::TrackType trackType ) const
{
    return m_tracks[trackType]->getClipPosition( uuid, trackId );
}

void
MainWorkflow::stop()
{
    /*
        Assume the method can be called without locking anything, since the workflow won't
        be queried by the renderer (When stopping the renderer, it stops its media player
        before stopping the mainworkflow.
    */
    m_renderStarted = false;
    for (unsigned int i = 0; i < Workflow::NbTrackType; ++i)
    {
        m_tracks[i]->stop();
        m_currentFrame[i] = 0;
    }
    emit frameChanged( 0, Vlmc::Renderer );
}

void
MainWorkflow::stopFrameComputing()
{
    for ( qint32 type = 0; type < Workflow::NbTrackType; ++type )
        m_tracks[type]->stopFrameComputing();
}

void
MainWorkflow::muteTrack( unsigned int trackId, Workflow::TrackType trackType )
{
    m_tracks[trackType]->muteTrack( trackId );
}

void
MainWorkflow::unmuteTrack( unsigned int trackId, Workflow::TrackType trackType )
{
    m_tracks[trackType]->unmuteTrack( trackId );
}

void
MainWorkflow::muteClip( const QUuid& uuid, unsigned int trackId,
                        Workflow::TrackType trackType )
{
    m_tracks[trackType]->muteClip( uuid, trackId );
}

void
MainWorkflow::unmuteClip( const QUuid& uuid, unsigned int trackId,
                          Workflow::TrackType trackType )
{
    m_tracks[trackType]->unmuteClip( uuid, trackId );
}

void
MainWorkflow::setCurrentFrame( qint64 currentFrame, Vlmc::FrameChangedReason reason )
{
    QWriteLocker    lock( m_currentFrameLock );

    for ( unsigned int i = 0; i < Workflow::NbTrackType; ++i)
        m_currentFrame[i] = currentFrame;
    emit frameChanged( currentFrame, reason );
}

ClipHelper*
MainWorkflow::getClipHelper( const QUuid &uuid, unsigned int trackId,
                                Workflow::TrackType trackType )
{
    return m_tracks[trackType]->getClipHelper( uuid, trackId );
}

/**
 *  \warning    The mainworkflow is expected to be already cleared by the ProjectManager
 */
bool
MainWorkflow::load(const QDomDocument& root )
{
    QDomElement     project = root.firstChildElement( "workflow" );
    if ( project.isNull() == true )
        return false;

    QDomElement elem = project.firstChildElement();

    while ( elem.isNull() == false )
    {
        bool    ok;

        unsigned int trackId = elem.attribute( "id" ).toUInt( &ok );
        if ( ok == false )
        {
            vlmcWarning() << "Invalid track number in project file";
            return false;
        }
        Workflow::TrackType     type;
        int utype = elem.attribute( "type" ).toInt( &ok );
        if ( ok == false || (utype < 0 && utype >= Workflow::NbTrackType ) )
        {
            vlmcWarning() << "Invalid track type";
            return false;
        }
        type = static_cast<Workflow::TrackType>( utype );

        track( type, trackId )->loadEffects( elem );

        QDomElement clips = elem.firstChildElement( "clips" );
        if ( clips.isNull() == false )
        {
            QDomElement clip = clips.firstChildElement();
            while ( clip.isNull() == false )
            {
                //Iterate over clip fields:
                QString                     uuid;
                QString                     begin;
                QString                     end;
                QString                     startFrame;
                QString                     chUuid;

                uuid = clip.attribute( "uuid" );
                begin = clip.attribute( "begin" );
                end = clip.attribute( "end" );
                startFrame = clip.attribute( "startFrame" );
                chUuid = clip.attribute( "helper" );

                if ( uuid.isEmpty() == true || startFrame.isEmpty() == true )
                {
                    vlmcWarning() << "Invalid clip node";
                    return false;
                }

                Clip* c = Core::getInstance()->library()->clip( uuid );
                if ( c != NULL )
                {
                    ClipHelper  *ch = new ClipHelper( c, begin.toLongLong(),
                                                      end.toLongLong(), chUuid );
                    track( type, trackId )->addClip( ch, startFrame.toLongLong() );

                    ch->clipWorkflow()->loadEffects( clip );
                }
                clip = clip.nextSiblingElement();
            }
        }
        elem = elem.nextSiblingElement();
    }
    return true;
}

bool
MainWorkflow::save( QXmlStreamWriter& project )
{
    project.writeStartElement( "workflow" );
    for ( unsigned int i = 0; i < Workflow::NbTrackType; ++i )
    {
        m_tracks[i]->save( project );
    }
    project.writeEndElement();
    return true;
}

void
MainWorkflow::clear()
{
    for ( unsigned int i = 0; i < Workflow::NbTrackType; ++i )
        m_tracks[i]->clear();
    emit cleared();
}

void
MainWorkflow::tracksEndReached()
{
    for ( unsigned int i = 0; i < Workflow::NbTrackType; ++i )
    {
        if ( m_tracks[i]->endIsReached() == false )
            return ;
    }
    emit mainWorkflowEndReached();
}

int
MainWorkflow::getTrackCount( Workflow::TrackType trackType ) const
{
    return m_tracks[trackType]->getTrackCount();
}

qint64
MainWorkflow::getCurrentFrame( bool lock /*= false*/ ) const
{
    if ( lock == true )
    {
        QReadLocker     lock( m_currentFrameLock );
        return m_currentFrame[Workflow::VideoTrack];
    }
    return m_currentFrame[Workflow::VideoTrack];
}

quint32
MainWorkflow::getWidth() const
{
    Q_ASSERT( m_width != 0 );
    return m_width;
}

quint32
MainWorkflow::getHeight() const
{
    Q_ASSERT( m_height != 0 );
    return m_height;
}

void
MainWorkflow::renderOneFrame()
{
    for ( unsigned int i = 0; i < Workflow::NbTrackType; ++i )
        m_tracks[i]->renderOneFrame();
    nextFrame( Workflow::VideoTrack );
    nextFrame( Workflow::AudioTrack );
}

void
MainWorkflow::setFullSpeedRender( bool val )
{
    for ( unsigned int i = 0; i < Workflow::NbTrackType; ++i )
        m_tracks[i]->setFullSpeedRender( val );
}

bool
MainWorkflow::contains( const QUuid &uuid ) const
{
    for ( qint32 type = 0; type < Workflow::NbTrackType; ++type )
        if ( m_tracks[type]->contains( uuid ) == true )
            return true;
    return false;
}

const Workflow::Frame*
MainWorkflow::blackOutput() const
{
    return m_blackOutput;
}

quint32
MainWorkflow::trackCount() const
{
    return m_trackCount;
}

void
MainWorkflow::lengthUpdated( qint64 )
{
    qint64  maxLength = 0;

    for ( unsigned int i = 0; i < Workflow::NbTrackType; ++i )
    {
        if ( m_tracks[i]->getLength() > maxLength )
            maxLength = m_tracks[i]->getLength();
    }
    if ( m_lengthFrame != maxLength )
    {
        m_lengthFrame = maxLength;
        emit lengthChanged( m_lengthFrame );
    }
}

TrackWorkflow*
MainWorkflow::track( Workflow::TrackType type, quint32 trackId )
{
    return m_tracks[type]->track( trackId );
}
