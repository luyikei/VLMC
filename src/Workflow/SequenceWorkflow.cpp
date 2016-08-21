/*****************************************************************************
 * SequenceWorkflow.cpp : Manages tracks in a single sequence
 *****************************************************************************
 * Copyright (C) 2008-2016 VideoLAN
 *
 * Authors: Yikei Lu    <luyikei.qmltu@gmail.com>
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

#include "SequenceWorkflow.h"

#include "Backend/MLT/MLTTrack.h"
#include "Backend/MLT/MLTMultiTrack.h"
#include "EffectsEngine/EffectHelper.h"
#include "Workflow/MainWorkflow.h"
#include "Main/Core.h"
#include "Library/Library.h"
#include "Tools/VlmcDebug.h"

SequenceWorkflow::SequenceWorkflow( size_t trackCount )
    : m_multitrack( new Backend::MLT::MLTMultiTrack )
    , m_trackCount( trackCount )
{
    for ( int i = 0; i < trackCount; ++i )
    {
        auto audioTrack = std::shared_ptr<Backend::ITrack>( new Backend::MLT::MLTTrack );
        audioTrack->setVideoEnabled( false );
        m_tracks[Workflow::AudioTrack] << audioTrack;

        auto videoTrack = std::shared_ptr<Backend::ITrack>( new Backend::MLT::MLTTrack );
        videoTrack->setMute( true );
        m_tracks[Workflow::VideoTrack] << videoTrack;

        auto multitrack = std::shared_ptr<Backend::IMultiTrack>( new Backend::MLT::MLTMultiTrack );
        multitrack->setTrack( *videoTrack, 0 );
        multitrack->setTrack( *audioTrack, 1 );
        m_multiTracks << multitrack;

        m_multitrack->setTrack( *multitrack, i );
    }
}

SequenceWorkflow::~SequenceWorkflow()
{
    delete m_multitrack;
    clear();
}

bool
SequenceWorkflow::addClip( QSharedPointer<Clip> const& clip, quint32 trackId, qint32 pos )
{
    auto ret = trackFromFormats( trackId, clip->formats() )->insertAt( *clip->input(), pos );
    if ( ret == false )
        return false;
    m_clips.insert( clip->uuid(), std::make_tuple( clip, trackId, pos ) );
    return true;
}

QString
SequenceWorkflow::addClip( const QUuid& uuid, quint32 trackId, qint32 pos, bool isAudioClip )
{
    auto newClip = Core::instance()->library()->clip( uuid );
    if ( newClip == nullptr )
    {
        vlmcCritical() << "Couldn't find an acceptable parent to be added.";
        return QUuid().toString();
    }

    if ( isAudioClip == true )
        newClip->setFormats( Clip::Audio );
    else
        newClip->setFormats( Clip::Video );

    bool ret = trackFromFormats( trackId, newClip->formats() )->insertAt( *newClip->input(), pos );
    if ( ret == false )
        return QUuid().toString();
    m_clips.insert( newClip->uuid(), std::make_tuple( newClip, trackId, pos ) );
    return newClip->uuid().toString();
}

bool
SequenceWorkflow::moveClip( const QUuid& uuid, quint32 trackId, qint64 pos )
{
    auto it = m_clips.find( uuid );
    if ( it == m_clips.end() )
    {
        vlmcCritical() << "Couldn't find a clip " << uuid;
        return false;
    }
    auto clip = std::get<ClipTupleIndex::Clip>( it.value() );
    auto oldTrackId = std::get<ClipTupleIndex::TrackId>( it.value() );
    auto oldPosition = std::get<ClipTupleIndex::Position>( it.value() );
    if ( oldPosition == pos )
        return true;
    auto track = trackFromFormats( oldTrackId, clip->formats() );
    bool ret = true;
    if ( trackId != oldTrackId )
    {
        removeClip( uuid );
        ret = addClip( clip, trackId, pos );
    }
    else
    {
        ret = track->move( std::get<ClipTupleIndex::Position>( it.value() ), pos );
        if ( ret == true )
        {
            m_clips.erase( it );
            m_clips.insert( uuid, std::make_tuple( clip, trackId, pos ) );
        }
    }
    // TODO: If we detect collision too strictly, there will be a problem if we want to move multiple
    //       clips at the same time.
    return ret;
}

bool
SequenceWorkflow::resizeClip( const QUuid& uuid, qint64 newBegin, qint64 newEnd, qint64 newPos )
{
    auto it = m_clips.find( uuid );
    if ( it == m_clips.end() )
    {
        vlmcCritical() << "Couldn't find a clip " << uuid;
        return false;
    }
    auto clip = std::get<ClipTupleIndex::Clip>( it.value() );
    auto trackId = std::get<ClipTupleIndex::TrackId>( it.value() );
    auto position = std::get<ClipTupleIndex::Position>( it.value() );
    auto track = trackFromFormats( trackId, clip->formats() );
    auto ret = track->resizeClip( track->clipIndexAt( position ), newBegin, newEnd );
    if ( ret == false )
        return false;
    ret = moveClip( uuid, trackId, newPos );
    return ret;
}

QSharedPointer<Clip>
SequenceWorkflow::removeClip( const QUuid& uuid )
{
    auto it = m_clips.find( uuid );
    if ( it == m_clips.end() )
    {
        vlmcCritical() << "Couldn't find a clip " << uuid;
        return {};
    }
    auto clip = std::get<ClipTupleIndex::Clip>( it.value() );
    auto trackId = std::get<ClipTupleIndex::TrackId>( it.value() );
    auto position = std::get<ClipTupleIndex::Position>( it.value() );
    auto track = trackFromFormats( trackId, clip->formats() );
    track->remove( track->clipIndexAt( position ) );
    m_clips.erase( it );
    clip->disconnect( this );
    return clip;

}

bool
SequenceWorkflow::linkClips( const QUuid& uuidA, const QUuid& uuidB )
{
    auto clipA = clip( uuidA );
    auto clipB = clip( uuidB );
    if ( !clipA || !clipB )
    {
        vlmcCritical() << "Couldn't find clips: " << uuidA << " and " << uuidB;
        return false;
    }
    clipA->setLinkedClipUuid( clipB->uuid() );
    clipB->setLinkedClipUuid( clipA->uuid() );
    clipA->setLinked( true );
    clipB->setLinked( true );
    return true;
}

bool
SequenceWorkflow::unlinkClips( const QUuid& uuidA, const QUuid& uuidB )
{
    auto clipA = clip( uuidA );
    auto clipB = clip( uuidB );
    if ( !clipA || !clipB )
    {
        vlmcCritical() << "Couldn't find clips: " << uuidA << " and " << uuidB;
        return false;
    }
    clipA->setLinked( false );
    clipB->setLinked( false );
    return true;
}

QVariant
SequenceWorkflow::toVariant() const
{
    QVariantList l;
    for ( auto it = m_clips.begin(); it != m_clips.end(); ++it )
    {
        auto clip = std::get<ClipTupleIndex::Clip>( it.value() );
        auto trackId = std::get<ClipTupleIndex::TrackId>( it.value() );
        auto position = std::get<ClipTupleIndex::Position>( it.value() );
        QVariantHash h {
            { "uuid", clip->uuid() },
            { "position", position },
            { "trackId", trackId },
            { "filters", EffectHelper::toVariant( clip->input() ) }
        };
        //FIXME: Missing linking informations
        l << h;
    }
    QVariantHash h{ { "clips", l }, { "filters", EffectHelper::toVariant( m_multitrack ) } };
    return h;
}

void
SequenceWorkflow::loadFromVariant( const QVariant& variant )
{
    for ( auto& var : variant.toMap()["clips"].toList() )
    {
        auto m = var.toMap();
        auto clip = Core::instance()->library()->clip( m["uuid"].toUuid() );

        if ( clip == nullptr )
        {
            vlmcCritical() << "Couldn't find an acceptable parent to be added.";
            continue;
        }

        addClip( clip, m["trackId"].toUInt(), m["position"].toLongLong() );

        auto isLinked = m["linked"].toBool();
        clip->setLinked( isLinked );
        if ( isLinked == true )
            clip->setLinkedClipUuid( m["linkedClip"].toString() );

        EffectHelper::loadFromVariant( m["filters"], clip->input() );

        emit Core::instance()->workflow()->clipAdded( clip->uuid().toString() );
    }
    EffectHelper::loadFromVariant( variant.toMap()["filters"], m_multitrack );
}

void
SequenceWorkflow::clear()
{
    auto it = m_clips.begin();
    while ( it != m_clips.end() )
    {
        removeClip( it.key() );
        // m_clips.begin() can be changed
        it = m_clips.begin();
    }
}

QSharedPointer<Clip>
SequenceWorkflow::clip( const QUuid& uuid )
{
    auto it = m_clips.find( uuid );
    if ( it == m_clips.end() )
        return {};
    return std::get<ClipTupleIndex::Clip>( it.value() );
}

quint32
SequenceWorkflow::trackId( const QUuid& uuid )
{
    auto it = m_clips.find( uuid );
    if ( it == m_clips.end() )
        return 0;
    return std::get<ClipTupleIndex::TrackId>( it.value() );
}

qint32
SequenceWorkflow::position( const QUuid& uuid )
{
    auto it = m_clips.find( uuid );
    if ( it == m_clips.end() )
        return 0;
    return std::get<ClipTupleIndex::Position>( it.value() );
}

Backend::IInput*
SequenceWorkflow::input()
{
    return m_multitrack;
}

Backend::IInput*
SequenceWorkflow::trackInput( quint32 trackId )
{
    Q_ASSERT( trackId < m_multiTracks.size() );
    return m_multiTracks[trackId].get();
}

std::shared_ptr<Backend::ITrack>
SequenceWorkflow::trackFromFormats( quint32 trackId, Clip::Formats formats )
{
    if ( trackId >= (quint32)m_trackCount )
        return nullptr;
    if ( formats.testFlag( Clip::Audio ) )
        return m_tracks[Workflow::AudioTrack][trackId];
    else if ( formats.testFlag( Clip::Video ) )
        return m_tracks[Workflow::VideoTrack][trackId];
    return nullptr;
}
