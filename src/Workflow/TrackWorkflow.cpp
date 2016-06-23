/*****************************************************************************
 * TrackWorkflow.cpp : Will query the Clip workflow for each successive clip in the track
 *****************************************************************************
 * Copyright (C) 2008-2016 VideoLAN
 *
 * Authors: Yikei Lu    <luyikei.qmltu@gmail.com>
 *          Hugo Beauzée-Luyssen <hugo@beauzee.fr>
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


#include "TrackWorkflow.h"

#include "Project/Project.h"
#include "Media/Clip.h"
#include "EffectsEngine/EffectHelper.h"
#include "Backend/VLC/VLCSource.h"
#include "Backend/MLT/MLTTrack.h"
#include "Backend/MLT/MLTTractor.h"
#include "Main/Core.h"
#include "Library/Library.h"
#include "MainWorkflow.h"
#include "Media/Media.h"
#include "Types.h"
#include "vlmc.h"
#include "Tools/VlmcDebug.h"


#include <QReadLocker>
#include <QMutex>

TrackWorkflow::TrackWorkflow( quint32 trackId, Backend::ITractor* tractor ) :
        m_length( 0 ),
        m_trackId( trackId )
{
    m_clipsLock = new QReadWriteLock;
    m_mixerBuffer = new Workflow::Frame;

    m_track = new Backend::MLT::MLTTrack;
    tractor->setTrack( *m_track, trackId );

    for ( int i = 0; i < Workflow::NbTrackType; ++i )
        m_lastFrame[i] = 0;

    connect( this, SIGNAL( effectAdded( EffectHelper*, qint64 ) ),
             this, SLOT( __effectAdded( EffectHelper*, qint64) ) );
    connect( this, SIGNAL( effectMoved( EffectHelper*, qint64 ) ),
             this, SLOT( __effectMoved( EffectHelper*, qint64 ) ) );
    connect( this, SIGNAL( effectRemoved( QUuid ) ),
             this, SLOT( __effectRemoved(QUuid ) ) );
}

TrackWorkflow::~TrackWorkflow()
{
    delete m_track;
    delete m_mixerBuffer;
    delete m_clipsLock;
}

void
TrackWorkflow::addClip( Clip* clip, qint64 start )
{
    // Avoid adding an audio track of a video for now since a video track will be enough for MLT.
    if ( !( clip->media()->producer()->hasVideo() && clip->formats().testFlag( Clip::Audio ) ) )
    {
        m_track->insertAt( *clip->producer(), start );
        m_clips.insert( start, clip );
    }
    computeLength();
}

//Must be called from a thread safe method (m_clipsLock locked)
void
TrackWorkflow::computeLength()
{
    bool    changed = false;
    if ( m_clips.count() == 0 )
    {
        if ( m_length != 0 )
            changed = true;
        m_length = 0;
    }
    else
    {
        auto it = m_clips.end() - 1;
        qint64  newLength = it.key() + it.value()->length();
        if ( m_length != newLength )
            changed = true;
        m_length = newLength;
    }
    if ( changed == true )
        emit lengthChanged( m_length );
}

qint64
TrackWorkflow::getLength() const
{
    return m_length;
}

qint64
TrackWorkflow::getClipPosition( const QUuid& uuid ) const
{
    auto     it = m_clips.begin();
    auto     end = m_clips.end();

    while ( it != end )
    {
        if ( it.value()->uuid() == uuid )
            return it.key();
        ++it;
    }
    return -1;
}

Clip*
TrackWorkflow::clip( const QUuid& uuid )
{
    auto     it = m_clips.begin();
    auto     end = m_clips.end();

    while ( it != end )
    {
        if ( it.value()->uuid() == uuid )
            return it.value();
        ++it;
    }
    return nullptr;
}

bool
TrackWorkflow::hasNoMoreFrameToRender( qint64 currentFrame ) const
{
    if ( m_clips.size() == 0 )
        return true;
    //This is the last video by clipronological order :
    auto   it = m_clips.end() - 1;
    auto   clip = it.value();
    //If it ends before the current frame, we reached end.
    return ( clip->length() + it.key() < currentFrame );
}

void
TrackWorkflow::stop()
{
    /* TODO
    auto       it = m_clips.begin();
    auto       end = m_clips.end();

    while ( it != end )
    {
        stopClipWorkflow( it.value() );
        ++it;
    }
    for ( int i = 0; i < Workflow::NbTrackType; ++i )
        m_lastFrame[i] = 0;
    m_isRendering = false;*/
}

void
TrackWorkflow::moveClip( const QUuid& id, qint64 startingFrame )
{
    QWriteLocker    lock( m_clipsLock );

    auto       it = m_clips.begin();
    auto       end = m_clips.end();

    while ( it != end )
    {
        if ( it.value()->uuid() == id )
        {
            auto clip = it.value();

            auto producer = m_track->clipAt( it.key() );
            m_track->remove( m_track->clipIndexAt( it.key() ) );
            m_track->insertAt( *producer, startingFrame );
            delete producer;

            m_clips.erase( it );
            m_clips.insert( startingFrame, clip );
            computeLength();
            emit clipMoved( this, clip->uuid(), startingFrame );
            return ;
        }
        ++it;
    }
}

void
TrackWorkflow::clipDestroyed( const QUuid& id )
{
    QWriteLocker    lock( m_clipsLock );

    auto       it = m_clips.begin();
    auto       end = m_clips.end();

    while ( it != end )
    {
        if ( it.value()->uuid() == id )
        {
            auto   clip = it.value();
            m_clips.erase( it );
            computeLength();
            clip->disconnect( this );
            emit clipRemoved( this, id );
            return ;
        }
        ++it;
    }
}

Clip*
TrackWorkflow::removeClip( const QUuid& id )
{
    QWriteLocker    lock( m_clipsLock );

    auto       it = m_clips.begin();
    auto       end = m_clips.end();

    while ( it != end )
    {
        if ( it.value()->uuid() == id )
        {
            auto    clip = it.value();
            m_track->remove( m_track->clipIndexAt( it.key() ) );
            m_clips.erase( it );
            computeLength();
            clip->disconnect( this );
            emit clipRemoved( this, clip->uuid() );
            return clip;
        }
        ++it;
    }
    return nullptr;
}

QVariant
TrackWorkflow::toVariant() const
{
    QVariantList l;
    for ( auto it = m_clips.cbegin(); it != m_clips.cend(); it++ )
    {
        auto    clip = it.value();
        l << QVariantHash{
                    { "clip", clip->uuid() },
                    { "begin", clip->begin() },
                    { "end", clip->end() },
                    { "startFrame", it.key() },
                    { "filters", clip->toVariant() }
                };
    }
    QVariantHash h{ { "clips", l } };
    return QVariant( h );
}

void
TrackWorkflow::loadFromVariant( const QVariant &variant )
{
    for ( auto& var : variant.toMap()[ "clips" ].toList() )
    {
        QVariantMap m = var.toMap();
        const QString& uuid     = m["clip"].toString();
        qint64 startFrame       = m["startFrame"].toLongLong();
        qint64 begin            = m["begin"].toLongLong();
        qint64 end              = m["end"].toLongLong();

        if ( uuid.isEmpty() )
        {
            vlmcWarning() << "Invalid clip node";
            return ;
        }

        Clip  *clip = Core::instance()->workflow()->createClip( QUuid( uuid ) );
        if ( clip == nullptr )
            continue ;
        clip->setBoundaries( begin, end );
        addClip( clip, startFrame );

        // TODO clip->clipWorkflow()->loadFromVariant( m["filters"] );
    }
}

void
TrackWorkflow::clear()
{
    QWriteLocker    lock( m_clipsLock );
    m_clips.clear();
    m_length = 0;
}

void
TrackWorkflow::adjustClipTime( qint64 currentFrame, qint64 start, Clip* c )
{
    float fps = c->media()->producer()->fps();
    qint64  nbMs = ( currentFrame - start ) / fps * 1000;
    qint64  beginInMs = c->begin() / fps * 1000;
    qint64  startFrame = beginInMs + nbMs;
    // TODO cw->setTime( startFrame );
}

void
TrackWorkflow::renderOneFrame()
{
    m_renderOneFrame.store( true );
}

void
TrackWorkflow::setFullSpeedRender( bool val )
{
    /* TODO
    foreach ( ClipWorkflow* cw, m_clips.values() )
    {
        cw->setFullSpeedRender( val );
    }
    */
}

void
TrackWorkflow::muteClip( const QUuid &uuid )
{
    /* TODO
    QWriteLocker    lock( m_clipsLock );

    auto       it = m_clips.begin();
    auto       end = m_clips.end();

    while ( it != end )
    {
        if ( it.value()->uuid() == uuid )
        {
            it.value()->mute();
            return ;
        }
        ++it;
    }
    vlmcWarning() << "Failed to mute clip" << uuid << "it probably doesn't exist "
            "in this track";
            */
}

void
TrackWorkflow::unmuteClip( const QUuid &uuid )
{
    /* TODO
    QWriteLocker    lock( m_clipsLock );

    auto       it = m_clips.begin();
    auto       end = m_clips.end();

    while ( it != end )
    {
        if ( it.value()->uuid() == uuid )
        {
            it.value()->unmute();
            return ;
        }
        ++it;
    }
    vlmcWarning() << "Failed to unmute clip" << uuid << "it probably doesn't exist "
            "in this track";
            */
}

void
TrackWorkflow::initRender( quint32 width, quint32 height )
{
    QReadLocker     lock( m_clipsLock );

    m_mixerBuffer->resize( width * height * Workflow::Depth );
    m_width = width;
    m_height = height;
    m_isRendering = true;
    /* TODO
    auto       it = m_clips.begin();
    auto       end = m_clips.end();
    while ( it != end )
    {
        qint64          start = it.key();
        ClipWorkflow*   cw = it.value();
        if ( start < TrackWorkflow::nbFrameBeforePreload )
            preloadClip( cw );
        ++it;
    }
    initFilters();
    initMixers();
    */
}

bool
TrackWorkflow::contains( const QUuid &uuid ) const
{
    auto       it = m_clips.begin();
    auto       end = m_clips.end();

    while ( it != end )
    {
        if ( it.value()->uuid() == uuid ||
             it.value()->isChild( uuid ) )
            return true;
        ++it;
    }
    return false;
}

void
TrackWorkflow::stopFrameComputing()
{
    /* TODO
    auto       it = m_clips.begin();
    auto       end = m_clips.end();

    while ( it != end )
    {
        ClipWorkflow*   cw = it.value();

        ClipWorkflow::State state = cw->getState();
        if ( state == ClipWorkflow::Stopped ||
             state == ClipWorkflow::Error )
        {
            return ;
        }
        cw->stop();
        ++it;
    }
    */
}

quint32
TrackWorkflow::trackId() const
{
    return m_trackId;
}

EffectsEngine::EffectList*
TrackWorkflow::filters()
{
    return &m_filters;
}

EffectsEngine::EffectList*
TrackWorkflow::mixers()
{
    return &m_mixers;
}

void
TrackWorkflow::__effectAdded( EffectHelper* helper, qint64 pos )
{
    /* TODO
    if ( helper->target()->effectType() == ClipEffectUser )
    {
        ClipWorkflow    *cw = qobject_cast<ClipWorkflow*>( helper->target() );
        Q_ASSERT( cw != nullptr );
        pos += getClipPosition( cw->clip()->uuid() );
    }
    emit effectAdded( this, helper, pos );
    */
}

void
TrackWorkflow::__effectRemoved( const QUuid& uuid )
{
    emit effectRemoved( this, uuid );
}

void
TrackWorkflow::__effectMoved( EffectHelper* helper, qint64 pos )
{
    /* TODO
    if ( helper->target()->effectType() == ClipEffectUser )
    {
        ClipWorkflow    *cw = qobject_cast<ClipWorkflow*>( helper->target() );
        Q_ASSERT( cw != nullptr );
        pos += getClipPosition( cw->clip()->uuid() );
    }
    emit effectMoved( this, helper->uuid(), pos );
    */
}

qint64
TrackWorkflow::length() const
{
    return m_length;
}

EffectUser::Type
TrackWorkflow::effectType() const
{
    return TrackEffectUser;
}

Backend::IProducer*
TrackWorkflow::producer()
{
    return m_track;
}
