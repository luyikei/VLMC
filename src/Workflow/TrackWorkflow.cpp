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
#include "Backend/MLT/MLTTrack.h"
#include "Backend/MLT/MLTMultiTrack.h"
#include "Main/Core.h"
#include "Library/Library.h"
#include "MainWorkflow.h"
#include "Media/Media.h"
#include "Types.h"
#include "vlmc.h"
#include "Tools/VlmcDebug.h"


#include <QReadLocker>
#include <QMutex>

TrackWorkflow::TrackWorkflow( quint32 trackId, Backend::IMultiTrack* multitrack ) :
        m_trackId( trackId )
{
    m_clipsLock = new QReadWriteLock;

    auto audioTrack = new Backend::MLT::MLTTrack;
    audioTrack->setVideoEnabled( false );
    m_audioTrack = audioTrack;

    auto videoTrack = new Backend::MLT::MLTTrack;
    videoTrack->setMute( true );
    m_videoTrack = videoTrack;

    m_multitrack = new Backend::MLT::MLTMultiTrack;
    m_multitrack->setTrack( *m_videoTrack, 0 );
    m_multitrack->setTrack( *m_audioTrack, 1 );

    multitrack->setTrack( *m_multitrack, trackId );
}

TrackWorkflow::~TrackWorkflow()
{
    delete m_audioTrack;
    delete m_videoTrack;
    delete m_multitrack;
    delete m_clipsLock;
}

inline Backend::ITrack*
TrackWorkflow::trackFromFormats( Clip::Formats formats )
{
    if ( formats.testFlag( Clip::Audio ) )
        return m_audioTrack;
    else if ( formats.testFlag( Clip::Video ) )
        return m_videoTrack;
    return nullptr;
}

void
TrackWorkflow::addClip( std::shared_ptr<Clip> const& clip, qint64 start )
{
    trackFromFormats( clip->formats() )->insertAt( *clip->input(), start );
    m_clips.insertMulti( start, clip );
    emit clipAdded( this, clip, start );
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

std::shared_ptr<Clip>
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
            auto track = trackFromFormats( it.value()->formats() );
            auto input = track->clipAt( it.key() );
            track->remove( track->clipIndexAt( it.key() ) );
            track->insertAt( *input, startingFrame );
            delete input;

            m_clips.erase( it );
            m_clips.insertMulti( startingFrame, clip );
            emit clipMoved( this, clip->uuid(), startingFrame );
            return ;
        }
        ++it;
    }
}

void
TrackWorkflow::resizeClip( const QUuid &id, qint64 begin, qint64 end )
{
    QWriteLocker    lock( m_clipsLock );

    for ( auto it = m_clips.begin(); it != m_clips.end(); ++it )
    {
        auto clip = it.value();
        if ( clip->uuid() == id )
        {
            auto track = trackFromFormats( clip->formats() );
            track->resizeClip( track->clipIndexAt( it.key() ), begin, end );
        }
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
            clip->disconnect( this );
            emit clipRemoved( this, id );
            return ;
        }
        ++it;
    }
}

std::shared_ptr<Clip>
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
            auto    track = trackFromFormats( clip->formats() );
            track->remove( track->clipIndexAt( it.key() ) );
            m_clips.erase( it );
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
    for ( auto it = m_clips.begin(); it != m_clips.end(); ++it )
    {
        auto    clip = it.value();
        QVariantHash h;
        h.insert( "parent", clip->parent()->uuid().toString() );
        h.insert( "begin", clip->begin() );
        h.insert( "end", clip->end() );
        h.insert( "formats", (int)clip->formats() );
        h.insert( "filters", EffectHelper::toVariant( clip->input() ) );
        h.insert( "startFrame", it.key() );
        l << h;
    }
    QVariantHash h{ { "clips", l }, { "filters", EffectHelper::toVariant( m_multitrack ) } };
    return QVariant( h );
}

void
TrackWorkflow::loadFromVariant( const QVariant &variant )
{
    for ( auto& var : variant.toMap()[ "clips" ].toList() )
    {
        auto m = var.toMap();
        auto c = std::shared_ptr<Clip>( Core::instance()->workflow()->createClip( QUuid( m["parent"].toString() ) ) );
        c->setBoundaries( m["begin"].toULongLong(),
                          m["end"].toULongLong()
                         );
        c->setFormats( (Clip::Formats)m["formats"].toInt() );
        EffectHelper::loadFromVariant( m["filters"], c->input() );
        addClip( c, m["startFrame"].toLongLong() );
    }
    EffectHelper::loadFromVariant( variant.toMap()["filters"], m_multitrack );
}

void
TrackWorkflow::clear()
{
    QWriteLocker    lock( m_clipsLock );
    m_clips.clear();
}

void
TrackWorkflow::mute( bool muted, Workflow::TrackType trackType )
{
    if ( trackType == Workflow::AudioTrack )
        m_audioTrack->setMute( muted );
    else
        m_videoTrack->setVideoEnabled( !muted );
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

quint32
TrackWorkflow::trackId() const
{
    return m_trackId;
}

Backend::IInput*
TrackWorkflow::input()
{
    return m_multitrack;
}
