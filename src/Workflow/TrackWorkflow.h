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

class   Clip;
class   Clip;
class   ClipWorkflow;
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

template <typename T>
class   QList;
class   QMutex;
class   QReadWriteLock;
class   QWaitCondition;

class   TrackWorkflow : public QObject
{
    Q_OBJECT

    public:
        TrackWorkflow( quint32 trackId, Backend::ITractor* tractor );
        ~TrackWorkflow();

        qint64                                  getLength() const;
        void                                    stop();
        void                                    moveClip( const QUuid& id, qint64 startingFrame );
        Clip*                                   removeClip( const QUuid& id );
        void                                    addClip( Clip*, qint64 start );
        void                                    addClip( ClipWorkflow*, qint64 start );
        qint64                                  getClipPosition( const QUuid& uuid ) const;
        Clip                                    *clip( const QUuid& uuid );

        //FIXME: this won't be reliable as soon as we change the fps from the configuration
        static const unsigned int               nbFrameBeforePreload = 60;

        virtual QVariant                        toVariant() const;
        void                                    loadFromVariant( const QVariant& variant );
        void                                    clear();

        void                                    renderOneFrame();

        /**
         *  \sa     MainWorkflow::setFullSpeedRender();
         */
        void                                    setFullSpeedRender( bool val );

        /**
         *  \brief      Mute a clip
         *
         *  Mutting a clip will prevent it to be rendered.
         *  \param  uuid    The uuid of the clip to mute.
         */
        void                                    muteClip( const QUuid& uuid );
        void                                    unmuteClip( const QUuid& uuid );

        void                                    initRender(quint32 width, quint32 height);

        bool                                    contains( const QUuid& uuid ) const;

        void                                    stopFrameComputing();
        bool                                    hasNoMoreFrameToRender( qint64 currentFrame ) const;
        quint32                                 trackId() const;
        virtual qint64                          length() const;

        Backend::IProducer*                     producer();

    private:
        void                                    computeLength();
        void                                    adjustClipTime( qint64 currentFrame, qint64 start, Clip* cw );

    private:
        Backend::ITrack*                        m_track;

        QMap<qint64, Clip*>                     m_clips;

        /**
         *  \brief      The track length in frames.
        */
        qint64                                  m_length;

        QAtomicInteger<bool>                    m_renderOneFrame;

        QReadWriteLock*                         m_clipsLock;

        qint64                                  m_lastFrame[Workflow::NbTrackType];
        Workflow::Frame                         *m_mixerBuffer;
        const quint32                           m_trackId;

    private slots:
        void                __effectAdded( EffectHelper*, qint64 );
        void                __effectRemoved( const QUuid& );
        void                __effectMoved( EffectHelper*, qint64 );
        void                clipDestroyed( const QUuid &uuid );

    signals:
        void                lengthChanged( qint64 newLength );
        void                clipAdded( TrackWorkflow*, Workflow::Helper*, qint64 );
        void                clipRemoved( TrackWorkflow*, const QUuid& );
        void                clipMoved( TrackWorkflow*, const QUuid&, qint64 );

        //these signals are here to ease connection with tracksview, as it only
        //monitors tracks, and not generics EffectUsers
        void                effectAdded( TrackWorkflow*, Workflow::Helper*, qint64 );
        void                effectRemoved( TrackWorkflow*, const QUuid& );
        void                effectMoved( TrackWorkflow*, const QUuid&, qint64 );

        void                effectAdded( EffectHelper *helper, qint64 pos );
        void                effectMoved( EffectHelper *helper, qint64 newPos );
        void                effectRemoved( const QUuid& );

};

#endif // TRACKWORKFLOW_H
