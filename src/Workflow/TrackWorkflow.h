/*****************************************************************************
 * TrackWorkflow.h : Will query the Clip workflow for each successive clip in the track
 *****************************************************************************
 * Copyright (C) 2008-2016 VideoLAN
 *
 * Authors: Yikei Lu        <luyikei.qmltu@gmail.com>
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

#ifndef TRACKWORKFLOW_H
#define TRACKWORKFLOW_H

#include "Types.h"

#include <QObject>
#include <QMap>
#include <QXmlStreamWriter>

#include "Media/Clip.h"

class   MainWorkflow;

class   EffectHelper;

namespace   Backend
{
class ITrack;
class ITractor;
class IProducer;
}

namespace   Workflow
{
    class   Helper;
}

class   QMutex;
class   QReadWriteLock;
class   QWaitCondition;

class   TrackWorkflow : public QObject
{
    Q_OBJECT

    public:
        TrackWorkflow( quint32 trackId, Backend::ITractor* tractor );
        ~TrackWorkflow();

        Backend::ITrack*                        trackFromFormats( Clip::Formats formats );

        void                                    moveClip( const QUuid& id, qint64 startingFrame );
        void                                    resizeClip( const QUuid& id, qint64 begin, qint64 end );
        Clip*                                   removeClip( const QUuid& id );
        void                                    addClip( Clip*, qint64 start );
        qint64                                  getClipPosition( const QUuid& uuid ) const;
        Clip                                    *clip( const QUuid& uuid );

        virtual QVariant                        toVariant() const;
        void                                    loadFromVariant( const QVariant& variant );
        void                                    clear();

        /**
         *  \brief      Mute a clip
         *
         *  Mutting a clip will prevent it to be rendered.
         *  \param  uuid    The uuid of the clip to mute.
         */
        void                                    muteClip( const QUuid& uuid );
        void                                    unmuteClip( const QUuid& uuid );

        bool                                    contains( const QUuid& uuid ) const;

        quint32                                 trackId() const;

        Backend::IProducer*                     producer();

    private:
        Backend::ITractor*                      m_tractor;
        Backend::ITrack*                        m_audioTrack;
        Backend::ITrack*                        m_videoTrack;

        QMap<qint64, Clip*>                     m_clips;

        QReadWriteLock*                         m_clipsLock;
        const quint32                           m_trackId;

    private slots:
        void                clipDestroyed( const QUuid &uuid );

    signals:
        void                lengthChanged( qint64 newLength );
        void                clipAdded( TrackWorkflow*, Workflow::Helper*, qint64 );
        void                clipRemoved( TrackWorkflow*, const QUuid& );
        void                clipMoved( TrackWorkflow*, const QUuid&, qint64 );

        //these signals are here to ease connection with tracksview, as it only
        //monitors tracks
        void                effectAdded( TrackWorkflow*, Workflow::Helper*, qint64 );
        void                effectRemoved( TrackWorkflow*, const QUuid& );
        void                effectMoved( TrackWorkflow*, const QUuid&, qint64 );

};

#endif // TRACKWORKFLOW_H
