/*****************************************************************************
 * MainWorkflow.h : Will query all of the track workflows to render the final
 *                  image
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

#ifndef MAINWORKFLOW_H
#define MAINWORKFLOW_H

#include "Types.h"
#include "Tools/Toggleable.hpp"

#include <memory>

class   Clip;
class   EffectsEngine;
class   Effect;
class   TrackWorkflow;
class   AbstractRenderer;

namespace Backend
{
class IMultiTrack;
class IInput;
}

class   Settings;
class   MediaContainer;

#include <QObject>
#include <QUuid>
#include <QMap>

/**
 *  \class  Represent the Timeline backend.
 */
class   MainWorkflow : public QObject
{
    Q_OBJECT

    public:
        MainWorkflow( Settings* projectSettings, int trackCount = 64 );
        ~MainWorkflow();

        /**
         *  \brief              Return the given clip position.
         *
         *  \param      uuid        The clip uuid
         *  \param      trackId     The track containing the clip
         *  \param      trackType   The type of the track containing the clip
         *  \return                 The given clip position, in frame. If not found, -1
         *                          is returned.
         */
        qint64                  getClipPosition( const QUuid& uuid, unsigned int trackId ) const;

        /**
         *  \brief      Mute a track.
         *
         *  A muted track will not be asked for render. It won't even emit endReached
         *  signal. To summerize, a mutted track is an hard deactivated track.
         *  \param  trackId     The id of the track to mute
         *  \param  trackType   The type of the track to mute.
         *  \sa     unmuteTrack( unsigned int, Workflow::TrackType );
         */
        void                    muteTrack( unsigned int trackId, Workflow::TrackType trackType );
        /**
         *  \brief      Unmute a track.
         *
         *  \param  trackId     The id of the track to unmute
         *  \param  trackType   The type of the track to unmute.
         *  \sa     muteTrack( unsigned int, Workflow::TrackType );
         */
        void                    unmuteTrack( unsigned int trackId, Workflow::TrackType trackType );

        /**
         *  \brief      Mute a clip.
         *
         *  \param  uuid        The clip's uuid.
         *  \param  trackId     The id of the track containing the clip.
         *  \param  trackType   The type of the track containing the clip.
         */
        void                    muteClip( const QUuid& uuid, unsigned int trackId );

        /**
         *  \brief      Unmute a clip.
         *
         *  \param  uuid        The clip's uuid.
         *  \param  trackId     The id of the track containing the clip.
         *  \param  trackType   The type of the track containing the clip.
         */
        void                    unmuteClip( const QUuid& uuid, unsigned int trackId );

        /**
         *  \brief              Get the number of track for a specific type
         *
         *  \param  trackType   The type of the tracks to count
         *  \return             The number of track for the type trackType
         */
        int                     getTrackCount() const;

        /**
         *  \return     true if the current workflow contains the clip which the uuid was
         *              passed. Falsed otherwise.
         *
         *  \param      uuid    The Clip uuid, not the Clip's.
         */
        bool                    contains( const QUuid& uuid ) const;

        TrackWorkflow           *track( quint32 trackId );

        /**
         * \brief   Return the number of track for each track type.
         */
        quint32                 trackCount() const;

        /**
         * \brief       Create a clip from a parent clip's uuid.
         *
         * The clip will be added to this MediaContainer, not to the Library.
         * The parent clip should be in the Library.
         */
        std::shared_ptr<Clip>   createClip( const QUuid& uuid, quint32 trackId );



        Q_INVOKABLE
        QString                 addClip( const QString& uuid, quint32 trackId, qint32 pos, bool isAudioClip );

        Q_INVOKABLE
        QJsonObject             clipInfo( const QString& uuid );

        Q_INVOKABLE
        void                    moveClip( quint32 trackId, const QString& uuid, qint64 startFrame );

        Q_INVOKABLE
        void                    resizeClip( const QString& uuid, qint64 newBegin,
                                            qint64 newEnd, qint64 newPos );

        Q_INVOKABLE
        void                    removeClip( const QString& uuid );

        bool                    startRenderToFile( const QString& outputFileName, quint32 width, quint32 height,
                                                   double fps, const QString& ar, quint32 vbitrate, quint32 abitrate,
                                                   quint32 nbChannels, quint32 sampleRate );

        bool                    canRender();

        AbstractRenderer*       renderer();

    private:

        /**
         *  \param      uuid : The clip's uuid.
         *  \param      trackId : the track id
         *  \param      trackType : the track type (audio or video)
         *  \returns    The clip that matches the given UUID, or nullptr.
         */
        std::shared_ptr<Clip>                   clip( const QUuid& uuid, unsigned int trackId );

        void                    preSave();
        void                    postLoad();

    private:
        QList<TrackWorkflow*>           m_tracks;
        QMap<qint32, std::shared_ptr<Clip>>           m_clips;
        const quint32                   m_trackCount;

        Settings*                       m_settings;

        AbstractRenderer*               m_renderer;
        Backend::IMultiTrack*           m_multitrack;

    public slots:
        /**
         *  \brief      Clear the workflow.
         *
         *  Calling this method will cause every clip workflow to be deleted, along with
         *  the associated Clip.
         *  This method will emit cleared() signal once finished.
         *  \sa     reCip( const QUuid&, unsigned int, Workflow::TrackType )
         *  \sa     cleared()
         */
        void                            clear();

        void                            setPosition( qint64 newFrame );

    signals:
        /**
         *  \brief      Used to notify a change to the timeline and preview widget cursor
         *
         *  \param      newFrame    The new rendered frame
         *  \param      reason      The reason for clipanging frame. Usually, if emitted
         *                          from the MainWorkflow, this should be "Renderer"
         */
        void                    frameChanged( qint64 newFrame,
                                              Vlmc::FrameChangedReason reason );

        /**
         *  \brief  Emitted when workflow end is reached
         *
         *  Workflow end is reached when tracksEndReached() is called, and no more tracks
         *  are activated (ie. they all reached end)
         */
        void                    mainWorkflowEndReached();

        /**
         *  \brief  Emitted when the workflow is cleared.
         *
         *  \sa     clear();
         */
        void                    cleared();

        /**
         *  \brief  Emitted when the global length of the workflow changes.
         *
         *  \param  newLength   The new length, in frames
         */
        void                    lengthChanged( qint64 length );
};

#endif // MAINWORKFLOW_H
