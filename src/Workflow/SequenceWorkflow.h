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

        struct Clip
        {
            Clip() = default;
            Clip( QSharedPointer<::Clip> c, const QUuid& uuid, quint32 tId, qint64 p, bool isAudio );
            QSharedPointer<::Clip>  clip;
            QUuid                   uuid;
            quint32                 trackId;
            qint64                  pos;
            QVector<QUuid>          linkedClips;
            // true is this instance represents an audio track, false otherwise
            bool                    isAudio;
        };

        /**
         * @brief addClip   Adds a clip to the sequence workflow
         * @param clip      A library clip's UUID
         * @param trackId   The target track
         * @param pos       Clip's position in the track
         * @param uuid      The new clip instance UUID. If this is a default created UUID, a new UUID
         *                  will be generated.
         * @return          The given instance UUID, or the newly generated one, representing a new
         *                  clip instance in the sequence workflow.
         *                  This instance UUID must be used to manipulate this new clip instance
         */
        QUuid                   addClip( QSharedPointer<::Clip> clip, quint32 trackId, qint32 pos,
                                         const QUuid& uuid, bool isAudioClip );
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

        inline std::shared_ptr<Backend::ITrack>         track( quint32 trackId, bool audio );

        QMap<QUuid, QSharedPointer<Clip>>               m_clips;

        Backend::IMultiTrack*           m_multitrack;
        QList<std::shared_ptr<Backend::ITrack>>         m_tracks[Workflow::NbTrackType];
        QList<std::shared_ptr<Backend::IMultiTrack>>    m_multiTracks;
        const size_t                    m_trackCount;

    signals:
        void                    clipAdded( QString );
        void                    clipRemoved( QString );
        void                    clipLinked( QString, QString );
        void                    clipMoved( QString );
        void                    clipResized( QString );
};

#endif // SEQUENCEWORKFLOW_H
