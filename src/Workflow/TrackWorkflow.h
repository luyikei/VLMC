/*****************************************************************************
 * TrackWorkflow.h : Will query the Clip workflow for each successive clip in the track
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

#ifndef TRACKWORKFLOW_H
#define TRACKWORKFLOW_H

#include "EffectsEngine.h"
#include "Types.h"

#include <QObject>
#include <QMap>
#include <QXmlStreamWriter>

class   Clip;
class   ClipHelper;
class   ClipWorkflow;

class   QDomElement;
class   QDomElement;
template <typename T>
class   QList;
class   QMutex;
class   QReadWriteLock;
class   QWaitCondition;

class   TrackWorkflow : public QObject
{
    Q_OBJECT

    public:
        TrackWorkflow( Workflow::TrackType type, quint32 trackId );
        ~TrackWorkflow();

        Workflow::OutputBuffer                  *getOutput( qint64 currentFrame,
                                                           qint64 subFrame, bool paused );
        qint64                                  getLength() const;
        void                                    stop();
        void                                    moveClip( const QUuid& id, qint64 startingFrame );
        Clip*                                   removeClip( const QUuid& id );
        ClipWorkflow*                           removeClipWorkflow( const QUuid& id );
        void                                    addClip( ClipHelper*, qint64 start );
        void                                    addClip( ClipWorkflow*, qint64 start );
        EffectsEngine::EffectHelper             *addEffect( Effect *effect, const QUuid &uuid );
        qint64                                  getClipPosition( const QUuid& uuid ) const;
        ClipHelper                              *getClipHelper( const QUuid& uuid );

        //FIXME: this won't be reliable as soon as we change the fps from the configuration
        static const unsigned int               nbFrameBeforePreload = 60;

        void                                    save( QXmlStreamWriter& project ) const;
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

        void                                    initRender( quint32 width, quint32 height,
                                                            double fps );

        bool                                    contains( const QUuid& uuid ) const;

        void                                    stopFrameComputing();
        bool                                    hasNoMoreFrameToRender( qint64 currentFrame ) const;
        quint32                                 trackId() const;
        Workflow::TrackType                     type() const;

    private:
        void                                    computeLength();
        Workflow::OutputBuffer                  *renderClip( ClipWorkflow* cw, qint64 currentFrame,
                                                            qint64 start, bool needRepositioning,
                                                            bool renderOneFrame, bool paused );
        void                                    preloadClip( ClipWorkflow* cw );
        void                                    stopClipWorkflow( ClipWorkflow* cw );
        void                                    adjustClipTime( qint64 currentFrame, qint64 start, ClipWorkflow* cw );


    private:
        QMap<qint64, ClipWorkflow*>             m_clips;
        EffectsEngine::MixerList                m_mixers;

        /**
         *  \brief      The track length in frames.
        */
        qint64                                  m_length;

        bool                                    m_renderOneFrame;
        QMutex                                  *m_renderOneFrameMutex;

        QReadWriteLock*                         m_clipsLock;

        const Workflow::TrackType               m_trackType;
        qint64                                  m_lastFrame;
        Workflow::Frame                         *m_mixerBuffer;
        double                                  m_fps;
        const quint32                           m_trackId;

    signals:
        void                lengthChanged( qint64 newLength );
        void                clipAdded( TrackWorkflow*, ClipHelper*, qint64 );
        void                clipRemoved( TrackWorkflow*, ClipHelper* );
};

#endif // TRACKWORKFLOW_H
