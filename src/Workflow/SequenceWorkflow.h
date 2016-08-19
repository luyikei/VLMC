/*****************************************************************************
 * SequenceWorkflow.h : Manages tracks in a single sequence
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

#ifndef SEQUENCEWORKFLOW_H
#define SEQUENCEWORKFLOW_H

#include <memory>
#include <tuple>

#include <QUuid>
#include <QMap>

#include "Media/Clip.h"
#include "Types.h"

namespace Backend
{
class IMultiTrack;
class ITrack;
class IInput;
}

namespace ClipTupleIndex
{
    enum {
        Clip,
        TrackId,
        Position
    };
}

class SequenceWorkflow : public QObject
{
    Q_OBJECT

    public:
        SequenceWorkflow( size_t trackCount = 64 );
        ~SequenceWorkflow();

        // Clip, Track Id, and Position
        using ClipTuple = std::tuple<QSharedPointer<Clip>, quint32, qint64>;

        bool                    addClip( QSharedPointer<Clip> const& clip, quint32 trackId, qint32 pos );
        QString                 addClip( const QUuid& uuid, quint32 trackId, qint32 pos, bool isAudioClip );
        bool                    moveClip( const QUuid& uuid, quint32 trackId, qint64 pos );
        bool                    resizeClip( const QUuid& uuid, qint64 newBegin,
                                            qint64 newEnd, qint64 newPos );
        QSharedPointer<Clip>    removeClip( const QUuid& uuid );
        bool                    linkClips( const QUuid& uuidA, const QUuid& uuidB );
        bool                    unlinkClips( const QUuid& uuidA, const QUuid& uuidB );

        QVariant                toVariant() const;
        void                    loadFromVariant( const QVariant& variant );
        void                    clear();

        QSharedPointer<Clip>    clip( const QUuid& uuid );
        quint32                 trackId( const QUuid& uuid );
        qint32                  position( const QUuid& uuid );

        Backend::IInput*        input();
        Backend::IInput*        trackInput( quint32 trackId );

    private:

        inline std::shared_ptr<Backend::ITrack>         trackFromFormats( quint32 trackId, Clip::Formats formats );

        QMap<QUuid, ClipTuple>          m_clips;

        Backend::IMultiTrack*           m_multitrack;
        QList<std::shared_ptr<Backend::ITrack>>         m_tracks[Workflow::NbTrackType];
        QList<std::shared_ptr<Backend::IMultiTrack>>    m_multiTracks;
        const size_t                    m_trackCount;
};

#endif // SEQUENCEWORKFLOW_H
