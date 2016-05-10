/*****************************************************************************
 * MainWorkflow.cpp : Will query all of the track workflows to render the final
 *                    image
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

#include "vlmc.h"
#include "Project/Project.h"
#include "Media/Clip.h"
#include "ClipWorkflow.h"
#include "Library/Library.h"
#include "MainWorkflow.h"
#include "Project/Project.h"
#include "TrackWorkflow.h"
#include "Settings/Settings.h"
#include "Tools/VlmcDebug.h"
#include "Workflow/Types.h"

#include <QMutex>

MainWorkflow::MainWorkflow( Settings* projectSettings, int trackCount ) :
        m_mediaContainer( new MediaContainer ),
        m_blackOutput( nullptr ),
        m_lengthFrame( 0 ),
        m_renderStarted( false ),
        m_width( 0 ),
        m_height( 0 ),
        m_trackCount( trackCount ),
        m_settings( new Settings )
{
    m_currentFrameLock = new QReadWriteLock;

    for ( unsigned int i = 0; i < Workflow::NbTrackType; ++i )
        m_currentFrame[i] = 0;

    for ( int i = 0; i < trackCount; ++i )
    {
        Toggleable<TrackWorkflow*> track;
        m_tracks << track;
        m_tracks[i].setPtr( new TrackWorkflow( i ) );
        connect( m_tracks[i], SIGNAL( lengthChanged( qint64 ) ),
                 this, SLOT( lengthUpdated(qint64) ) );
    }

    m_settings->createVar( SettingValue::List, "tracks", QVariantList(), "", "", SettingValue::Nothing );
    connect( m_settings, &Settings::postLoad, this, &MainWorkflow::postLoad, Qt::DirectConnection );
    connect( m_settings, &Settings::preSave, this, &MainWorkflow::preSave, Qt::DirectConnection );
    projectSettings->addSettings( "Workspace", *m_settings );
}

MainWorkflow::~MainWorkflow()
{
    for ( auto track : m_tracks )
        delete track;
    m_tracks.clear();
    delete m_currentFrameLock;
    delete m_blackOutput;
    delete m_settings;
    delete m_mediaContainer;
}

void
MainWorkflow::computeLength()
{
    qint64      maxLength = 0;

    for ( auto track : m_tracks )
    {
        if ( track->getLength() > maxLength )
            maxLength = track->getLength();
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
    //Reinit the effects in case the width/height has change
    m_renderStarted = true;
    m_width = width;
    m_height = height;
    if ( m_blackOutput != nullptr )
        delete m_blackOutput;
    m_blackOutput = new Workflow::Frame( m_width, m_height );
    memset( m_blackOutput->buffer(), 0, m_blackOutput->size() );
    m_endReached = ( m_lengthFrame == 0 ) ? true : false;
    if ( m_lengthFrame > 0 )
        for ( auto track : m_tracks )
            track->initRender( width, height );
    computeLength();
}

const Workflow::Frame*
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

        bool        validTrack = false;
        Workflow::Frame  *ret = nullptr;

        for ( int i = m_trackCount - 1; i >= 0; --i )
        {
            if ( m_tracks[i].activated() == false || m_tracks[i]->hasNoMoreFrameToRender( currentFrame ) )
                continue ;
            validTrack = true;
            ret = m_tracks[i]->getOutput( trackType, currentFrame, subFrame, paused );
            if ( ret == nullptr )
                continue ;
            else
                break ;
        }
        if ( validTrack == false )
        {
            m_endReached = true;
            emit mainWorkflowEndReached();
        }

        if ( trackType == Workflow::VideoTrack )
        {
            if ( ret == nullptr )
                return m_blackOutput;
        }
        return ret;
    }
    return nullptr;
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
MainWorkflow::getClipPosition( const QUuid& uuid, unsigned int trackId ) const
{
    Q_ASSERT( trackId < m_trackCount );
    return m_tracks[trackId]->getClipPosition( uuid );
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
        m_currentFrame[i] = 0;
    for ( auto track : m_tracks )
        track->stop();
    emit frameChanged( 0, Vlmc::Renderer );
}

void
MainWorkflow::stopFrameComputing()
{
    for ( auto track : m_tracks )
        track->stopFrameComputing();
}

void
MainWorkflow::muteTrack( unsigned int trackId )
{
    Q_ASSERT( trackId < m_trackCount );
    m_tracks[trackId].deactivate();
}

void
MainWorkflow::unmuteTrack( unsigned int trackId )
{
    Q_ASSERT( trackId < m_trackCount );
    m_tracks[trackId].activate();
}

void
MainWorkflow::muteClip( const QUuid& uuid, unsigned int trackId )
{
    Q_ASSERT( trackId < m_trackCount );
    m_tracks[trackId]->muteClip( uuid );
}

void
MainWorkflow::unmuteClip( const QUuid& uuid, unsigned int trackId )
{
    Q_ASSERT( trackId < m_trackCount );
    m_tracks[trackId]->unmuteClip( uuid );
}

void
MainWorkflow::setCurrentFrame( qint64 currentFrame, Vlmc::FrameChangedReason reason )
{
    QWriteLocker    lock( m_currentFrameLock );

    for ( unsigned int i = 0; i < Workflow::NbTrackType; ++i)
        m_currentFrame[i] = currentFrame;
    emit frameChanged( currentFrame, reason );
}

Clip*
MainWorkflow::clip( const QUuid &uuid, unsigned int trackId )
{
    Q_ASSERT( trackId < m_trackCount );
    return m_tracks[trackId]->clip( uuid );
}

void
MainWorkflow::clear()
{
    for ( auto track : m_tracks )
        track->clear();
    emit cleared();
}

void
MainWorkflow::tracksEndReached()
{
    if ( m_endReached == false )
        return ;
    emit mainWorkflowEndReached();
}

int
MainWorkflow::getTrackCount() const
{
    return m_trackCount;
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
    for ( auto track : m_tracks )
        if ( track.activated() == true )
            track->renderOneFrame();
    nextFrame( Workflow::VideoTrack );
    nextFrame( Workflow::AudioTrack );
}

void
MainWorkflow::setFullSpeedRender( bool val )
{
    for ( auto track : m_tracks )
        track->setFullSpeedRender( val );
}

bool
MainWorkflow::contains( const QUuid &uuid ) const
{
    for ( auto track : m_tracks )
        if ( track->contains( uuid ) == true )
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
MainWorkflow::preSave()
{
    QVariantList l;
    for ( auto track : m_tracks )
        l << track->toVariant();
    m_settings->value( "tracks" )->set( l );
}

void
MainWorkflow::postLoad()
{
    QVariantList l = m_settings->value( "tracks" )->get().toList();
    for ( int i = 0; i < l.size(); ++i )
        m_tracks[i]->loadFromVariant( l[i] );
}

void
MainWorkflow::lengthUpdated( qint64 )
{
    qint64  maxLength = 0;

    for ( auto track : m_tracks )
    {
        if ( track->getLength() > maxLength )
            maxLength = track->getLength();
    }
    if ( m_lengthFrame != maxLength )
    {
        m_lengthFrame = maxLength;
        emit ( m_lengthFrame );
    }
}

TrackWorkflow*
MainWorkflow::track( quint32 trackId )
{
    return m_tracks[ trackId ];
}
