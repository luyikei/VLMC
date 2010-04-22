/*****************************************************************************
 * MainWorkflow.cpp : Will query all of the track workflows to render the final
 *                    image
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

#include <QtDebug>

#include "vlmc.h"
#include "Clip.h"
#include "EffectsEngine.h"
#include "Library.h"
#include "LightVideoFrame.h"
#include "MainWorkflow.h"
#include "TrackWorkflow.h"
#include "TrackHandler.h"
#include "SettingsManager.h"

#include <QDomElement>
#include <QXmlStreamWriter>

LightVideoFrame     *MainWorkflow::blackOutput = NULL;

MainWorkflow::MainWorkflow( int trackCount ) :
        m_lengthFrame( 0 ),
        m_renderStarted( false ),
        m_width( 0 ),
        m_height( 0 )
{
    m_currentFrameLock = new QReadWriteLock;
    m_renderStartedMutex = new QMutex;

    m_tracks = new TrackHandler*[MainWorkflow::NbTrackType];
    m_currentFrame = new qint64[MainWorkflow::NbTrackType];
    for ( unsigned int i = 0; i < MainWorkflow::NbTrackType; ++i )
    {
        MainWorkflow::TrackType trackType =
                (i == 0 ? MainWorkflow::VideoTrack : MainWorkflow::AudioTrack );
        m_tracks[i] = new TrackHandler( trackCount, trackType );
        connect( m_tracks[i], SIGNAL( tracksEndReached() ),
                 this, SLOT( tracksEndReached() ) );
        m_currentFrame[i] = 0;
    }
    m_outputBuffers = new OutputBuffers;
}

MainWorkflow::~MainWorkflow()
{
    delete m_renderStartedMutex;
    delete m_currentFrameLock;
    delete m_currentFrame;
    for ( unsigned int i = 0; i < MainWorkflow::NbTrackType; ++i )
        delete m_tracks[i];
    delete[] m_tracks;
    delete MainWorkflow::blackOutput;
}

void
MainWorkflow::addClip( ClipHelper *clipHelper, unsigned int trackId,
                                        qint64 start, MainWorkflow::TrackType trackType,
                                        bool informGui )
{
    m_tracks[trackType]->addClip( clipHelper, trackId, start );
    computeLength();
    //Inform the GUI
    if ( informGui == true )
        emit clipAdded( clipHelper, trackId, start, trackType );
}

void
MainWorkflow::computeLength()
{
    qint64      maxLength = 0;

    for ( unsigned int i = 0; i < MainWorkflow::NbTrackType; ++i )
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
MainWorkflow::startRender( quint32 width, quint32 height )
{
    m_renderStarted = true;
    m_width = width;
    m_height = height;
    if ( blackOutput != NULL )
        delete blackOutput;
    blackOutput = new LightVideoFrame( m_width, m_height );
    // FIX ME vvvvvv , It doesn't update meta info (nbpixels, nboctets, etc.
    memset( (*blackOutput)->frame.octets, 0, (*blackOutput)->nboctets );
    for ( unsigned int i = 0; i < MainWorkflow::NbTrackType; ++i )
        m_tracks[i]->startRender();
    computeLength();
}

MainWorkflow::OutputBuffers*
MainWorkflow::getOutput( TrackType trackType, bool paused )
{
    QMutexLocker        lock( m_renderStartedMutex );

    if ( m_renderStarted == true )
    {
        QReadLocker         lock2( m_currentFrameLock );

        void*   ret = m_tracks[trackType]->getOutput( m_currentFrame[VideoTrack],
                                        m_currentFrame[trackType], paused );
        if ( trackType == MainWorkflow::VideoTrack )
        {
            LightVideoFrame*    frame = static_cast<LightVideoFrame*>( ret );
            if ( frame == NULL )
                m_outputBuffers->video = MainWorkflow::blackOutput;
            else
                m_outputBuffers->video = frame;
        }
        else
        {
            m_outputBuffers->audio = static_cast<AudioClipWorkflow::AudioSample*>( ret );
        }
    }
    return m_outputBuffers;
}

void
MainWorkflow::nextFrame( MainWorkflow::TrackType trackType )
{
    QWriteLocker    lock( m_currentFrameLock );

    ++m_currentFrame[trackType];
    if ( trackType == MainWorkflow::VideoTrack )
        emit frameChanged( m_currentFrame[MainWorkflow::VideoTrack], Renderer );
}

void
MainWorkflow::previousFrame( MainWorkflow::TrackType trackType )
{
    QWriteLocker    lock( m_currentFrameLock );

    --m_currentFrame[trackType];
    if ( trackType == MainWorkflow::VideoTrack )
        emit frameChanged( m_currentFrame[MainWorkflow::VideoTrack], Renderer );
}

qint64
MainWorkflow::getLengthFrame() const
{
    return m_lengthFrame;
}

qint64
MainWorkflow::getClipPosition( const QUuid& uuid, unsigned int trackId,
                               MainWorkflow::TrackType trackType ) const
{
    return m_tracks[trackType]->getClipPosition( uuid, trackId );
}

void
MainWorkflow::stop()
{
    QMutexLocker    lock( m_renderStartedMutex );
    QWriteLocker    lock2( m_currentFrameLock );

    m_renderStarted = false;
    for (unsigned int i = 0; i < MainWorkflow::NbTrackType; ++i)
    {
        m_tracks[i]->stop();
        m_currentFrame[i] = 0;
    }
    emit frameChanged( 0, Renderer );
}

void
MainWorkflow::moveClip( const QUuid &clipUuid, unsigned int oldTrack,
                        unsigned int newTrack, qint64 startingFrame,
                        MainWorkflow::TrackType trackType,
                        bool undoRedoCommand /*= false*/ )
{
    m_tracks[trackType]->moveClip( clipUuid, oldTrack, newTrack, startingFrame );
    computeLength();

    if ( undoRedoCommand == true )
    {
        emit clipMoved( clipUuid, newTrack, startingFrame, trackType );
    }
}

Clip*
MainWorkflow::removeClip( const QUuid &uuid, unsigned int trackId,
                          MainWorkflow::TrackType trackType )
{
    Clip *clip = m_tracks[trackType]->removeClip( uuid, trackId );
    if ( clip != NULL )
    {
        computeLength();
        emit clipRemoved( uuid, trackId, trackType );
    }
    return clip;
}

void
MainWorkflow::muteTrack( unsigned int trackId, MainWorkflow::TrackType trackType )
{
    m_tracks[trackType]->muteTrack( trackId );
}

void
MainWorkflow::unmuteTrack( unsigned int trackId, MainWorkflow::TrackType trackType )
{
    m_tracks[trackType]->unmuteTrack( trackId );
}

void
MainWorkflow::muteClip( const QUuid& uuid, unsigned int trackId,
                        MainWorkflow::TrackType trackType )
{
    m_tracks[trackType]->muteClip( uuid, trackId );
}

void
MainWorkflow::unmuteClip( const QUuid& uuid, unsigned int trackId,
                          MainWorkflow::TrackType trackType )
{
    m_tracks[trackType]->unmuteClip( uuid, trackId );
}

void
MainWorkflow::setCurrentFrame( qint64 currentFrame, MainWorkflow::FrameChangedReason reason )
{
    QWriteLocker    lock( m_currentFrameLock );

    if ( m_renderStarted == true )
    {
        //Since any track can be reactivated, we reactivate all of them, and let them
        //disable themself if required.
        for ( unsigned int i = 0; i < MainWorkflow::NbTrackType; ++i)
            m_tracks[i]->activateAll();
    }
    for ( unsigned int i = 0; i < MainWorkflow::NbTrackType; ++i)
        m_currentFrame[i] = currentFrame;
    emit frameChanged( currentFrame, reason );
}

Clip*
MainWorkflow::getClip( const QUuid &uuid, unsigned int trackId,
                       MainWorkflow::TrackType trackType )
{
    return m_tracks[trackType]->getClip( uuid, trackId );
}

/**
 *  \warning    The mainworkflow is expected to be already cleared by the ProjectManager
 */
void
MainWorkflow::loadProject( const QDomElement &root )
{
    QDomElement     project = root.firstChildElement( "workflow" );
    if ( project.isNull() == true )
        return ;

    QDomElement elem = project.firstChild().toElement();

    while ( elem.isNull() == false )
    {
        bool    ok;

        unsigned int trackId = elem.attribute( "id" ).toUInt( &ok );
        if ( ok == false )
        {
            qWarning() << "Invalid track number in project file";
            return ;
        }
        MainWorkflow::TrackType     type;
        int utype = elem.attribute( "type" ).toInt( &ok );
        if ( ok == false || (utype < 0 && utype >= MainWorkflow::NbTrackType ) )
        {
            qWarning() << "Invalid track type";
            return ;
        }
        type = static_cast<MainWorkflow::TrackType>( utype );

        QDomElement clip = elem.firstChild().toElement();
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
                qWarning() << "Invalid clip node";
                return ;
            }

            Clip* c = Library::getInstance()->clip( uuid );
            if ( c != NULL )
            {
                ClipHelper  *ch = new ClipHelper( c, begin.toLongLong(),
                                                  end.toLongLong(), chUuid );
                addClip( ch, trackId, startFrame.toLongLong(), type, true );
            }
            clip = clip.nextSibling().toElement();
        }
        elem = elem.nextSibling().toElement();
    }
}

void
MainWorkflow::saveProject( QXmlStreamWriter& project ) const
{
    project.writeStartElement( "workflow" );
    for ( unsigned int i = 0; i < MainWorkflow::NbTrackType; ++i )
    {
        m_tracks[i]->save( project );
    }
    project.writeEndElement();
}

void
MainWorkflow::clear()
{
    for ( unsigned int i = 0; i < MainWorkflow::NbTrackType; ++i )
        m_tracks[i]->clear();
    emit cleared();
}

void
MainWorkflow::tracksEndReached()
{
    for ( unsigned int i = 0; i < MainWorkflow::NbTrackType; ++i )
    {
        if ( m_tracks[i]->endIsReached() == false )
            return ;
    }
    emit mainWorkflowEndReached();
}

int
MainWorkflow::getTrackCount( MainWorkflow::TrackType trackType ) const
{
    return m_tracks[trackType]->getTrackCount();
}

qint64
MainWorkflow::getCurrentFrame() const
{
    QReadLocker     lock( m_currentFrameLock );

    return m_currentFrame[MainWorkflow::VideoTrack];
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
    for ( unsigned int i = 0; i < MainWorkflow::NbTrackType; ++i )
        m_tracks[i]->renderOneFrame();
    nextFrame( VideoTrack );
    nextFrame( AudioTrack );
}

void
MainWorkflow::setFullSpeedRender( bool val )
{
    for ( unsigned int i = 0; i < MainWorkflow::NbTrackType; ++i )
        m_tracks[i]->setFullSpeedRender( val );
}

ClipHelper*
MainWorkflow::split( ClipHelper* toSplit, ClipHelper* newClip, quint32 trackId,
                     qint64 newClipPos, qint64 newClipBegin, MainWorkflow::TrackType trackType )
{
    QMutexLocker    lock( m_renderStartedMutex );

    if ( newClip == NULL )
        newClip = new ClipHelper( toSplit->clip(), newClipBegin, toSplit->end() );

    toSplit->setEnd( newClipBegin );
    addClip( newClip, trackId, newClipPos, trackType, true );
    return newClip;
}

void
MainWorkflow::resizeClip( ClipHelper* clipHelper, qint64 newBegin, qint64 newEnd, qint64 newPos,
                          quint32 trackId, MainWorkflow::TrackType trackType,
                                      bool undoRedoAction /*= false*/ )
{
    QMutexLocker    lock( m_renderStartedMutex );

    if ( newBegin != clipHelper->begin() )
    {
        moveClip( clipHelper->uuid(), trackId, trackId, newPos, trackType, undoRedoAction );
    }
    clipHelper->setBoundaries( newBegin, newEnd );
}

void
MainWorkflow::unsplit( ClipHelper* origin, ClipHelper* splitted, quint32 trackId,
                       MainWorkflow::TrackType trackType )
{
    QMutexLocker    lock( m_renderStartedMutex );

    removeClip( splitted->uuid(), trackId, trackType );
    origin->setEnd( splitted->end() );
}

bool
MainWorkflow::contains( const QUuid &uuid ) const
{
    for ( qint32 type = 0; type < NbTrackType; ++type )
        if ( m_tracks[type]->contains( uuid ) == true )
            return true;
    return false;
}
