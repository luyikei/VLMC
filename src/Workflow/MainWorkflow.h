/*****************************************************************************
 * MainWorkflow.h : Will query all of the track workflows to render the final
 *                  image
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

#ifndef MAINWORKFLOW_H
#define MAINWORKFLOW_H

#include "EffectsEngine/EffectsEngine.h"
#include <QXmlStreamWriter>
#include "Types.h"
#include "Tools/Toggleable.hpp"

class   Clip;
class   ClipHelper;
class   EffectsEngine;
class   Effect;
class   ProjectManager;
class   TrackHandler;
class   TrackWorkflow;
namespace   Workflow
{
    class   Frame;
    class   AudioSample;
}

class   Settings;
class   QMutex;
class   QReadWriteLock;

#include <QObject>
#include <QUuid>

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
         *  \brief      Initialize the workflow for the render.
         *
         *  \param      width   The width to use with this render session.
         *  \param      height  The height to use with this render session.
         *  This will basically activate all the tracks, newLengthso they can render.
         */
        void                    startRender( quint32 width, quint32 height );
        /**
         *  \brief      Gets a frame from the workflow
         *
         *  \return A pointer to an output buffer. This pointer must NOT be released.
         *          Both types of this output buffer are not guarantied to be filled.
         *          However, if VideoTrack was passed as trackType, the video member of
         *          the output buffer is guarantied to be filled. The same applies for
         *          AudioTrack
         *  \param  trackType   The type of track you wish to get the render from.
         *  \param  paused      The paused state of the renderer
         */
        const Workflow::OutputBuffer    *getOutput( Workflow::TrackType trackType, bool paused );
        /**
         *  \brief              Set the workflow position by the desired frame
         *  \param              currentFrame: The desired frame to render from
         *  \param              reason: The program part which required this frame change
                                        (to avoid cyclic events)
        */
        void                    setCurrentFrame( qint64 currentFrame,
                                                Vlmc::FrameChangedReason reason );

        /**
         *  \brief              Get the workflow length in frames.
         *  \return             Returns the global length of the workflow
         *                      in frames.
        */
        qint64                  getLengthFrame() const;

        /**
         *  \brief              Get the currently rendered frame.
         *  \param      lock    If true, m_currentFrameLock will be locked for read.
         *                      If false, it won't be locked. This is usefull when calling
         *                      getCurentFrame from underlying workflows
         *  \return             Returns the current frame.
         *  \warning            Locks the m_currentFrameLock ReadWriteLock, unless false is
         *                      passed.
         */
        qint64                  getCurrentFrame( bool lock = true ) const;

        /**
         *  \brief      Stops the rendering.
         *
         *  This will stop every ClipWorkflow that are currently rendering.
         *  Calling this methid will cause the workflow to return to frame 0, and emit
         *  the signal frameChanged(), with the reason: Renderer
         */
        void                    stop();

        /**
         *  \brief              Unconditionnaly switch to the next frame.
         *
         *  \param  trackType   The type of the frame counter to increment.
         *                      Though it seems odd to speak about frame for AudioTrack,
         *                      it's mainly a render position used for
         *                      synchronisation purpose.
         *  \sa         previousFrame( Workflow::TrackType );
         */
        void                    nextFrame( Workflow::TrackType trackType );
        /**
         *  \brief      Unconditionnaly switch to the previous frame.
         *
         *  \param      trackType   The type of the frame counter to decrement.
         *                          Though it seems odd to speak about frame for
         *                          AudioTrack, it's mainly a render position used for
         *                          synchronisation purpose.
         *  \sa         nextFrame( Workflow::TrackType );
         */
        void                    previousFrame( Workflow::TrackType trackType );

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
        void                    muteTrack( unsigned int trackId );
        /**
         *  \brief      Unmute a track.
         *
         *  \param  trackId     The id of the track to unmute
         *  \param  trackType   The type of the track to unmute.
         *  \sa     muteTrack( unsigned int, Workflow::TrackType );
         */
        void                    unmuteTrack( unsigned int trackId );

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
         *  \brief      Get the width used for rendering.
         *
         *  This value is used by the ClipWorkflow that generates the frames.
         *  If this value is edited in the preferences, it will only change after the
         *  current render has been stopped.
         *  \return     The width (in pixels) of the currently rendered frames
         *  \sa         getHeight()
         */
        quint32                getWidth() const;
        /**
         *  \brief      Get the height used for rendering.
         *
         *  This value is used by the ClipWorkflow that generates the frames.
         *  If this value is edited in the preferences, it will only change after the
         *  current render has been stopped.
         *  \return     The height (in pixels) of the currently rendered frames
         *  \sa         getWidth()
         */
        quint32                getHeight() const;

        /**
         *  \brief          Will render one frame only
         *
         *  It will change the ClipWorkflow frame getting mode from Get to Pop, just for
         *  one frame
         */
        void                    renderOneFrame();

        /**
         *  \brief              Set the render speed.
         *
         *  This will activate or deactivate vlc's pace-control
         *  \param  val         If true, ClipWorkflow will use no-pace-control
         *                      else, pace-control.
         */
        void                    setFullSpeedRender( bool val );

        /**
         *  \return     true if the current workflow contains the clip which the uuid was
         *              passed. Falsed otherwise.
         *
         *  \param      uuid    The Clip uuid, not the ClipHelper's.
         */
        bool                    contains( const QUuid& uuid ) const;

        /**
         *  \brief      Stop the frame computing process.
         *
         *  This will stop all the currently running ClipWorkflow.
         *  This is meant to be called just before the stop() method.
         */
        void                    stopFrameComputing();

        TrackWorkflow           *track( quint32 trackId );

        const Workflow::Frame   *blackOutput() const;

        /**
         * \brief   Return the number of track for each track type.
         */
        quint32                 trackCount() const;

    private:
        /**
         *  \brief  Compute the length of the workflow.
         *
         *  This is meant to be used internally, after an operation occurs on the workflow
         *  such as adding a clip, removing it...
         *  This method will update the attribute m_lengthFrame
         */
        void                    computeLength();

        /**
         *  \param      uuid : The clip helper's uuid.
         *  \param      trackId : the track id
         *  \param      trackType : the track type (audio or video)
         *  \returns    The clip helper that matches the given UUID, or nullptr.
         */
        ClipHelper*             getClipHelper( const QUuid& uuid, unsigned int trackId );

        void                    preSave();
        void                    postLoad();

    private:
        QList<Toggleable<TrackWorkflow*>>     m_tracks;
        /// Pre-filled buffer used when there's nothing to render
        Workflow::Frame         *m_blackOutput;

        /// Lock for the m_currentFrame atribute.
        QReadWriteLock*                 m_currentFrameLock;
        /**
         *  \brief  An array of currently rendered frame.
         *
         *  This must be indexed with Workflow::TrackType.
         *  The Audio array entry is designed to synchronize the renders internally, as it
         *  is not actually a frame.
         *  If you wish to know which frame is really rendered, you must use
         *  m_currentFrame[MainWorkflow::VideoTrack], which is the value that will be used
         *  when setCurrentFrame() is called.
         */
        qint64                          m_currentFrame[Workflow::NbTrackType];
        /// The workflow length, in frame.
        qint64                          m_lengthFrame;
        /// This boolean describe is a render has been started
        bool                            m_renderStarted;

        /// Width used for the render
        quint32                         m_width;
        /// Height used for the render
        quint32                         m_height;
        /// Store the number of track for each track type.
        const quint32                   m_trackCount;

        bool                            m_endReached;

        Settings*                       m_settings;

    private slots:
        /**
         *  \brief  Called when a track end is reached
         *
         *  If all track has reached end, the mainWorkflowEndReached() signal will be
         *  emitted;
         *  \sa     mainWorkflowEndReached()
         */
        void                            tracksEndReached();

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

        void                            lengthUpdated( qint64 lengthUpdated );

    signals:
        /**
         *  \brief      Used to notify a change to the timeline and preview widget cursor
         *
         *  \param      newFrame    The new rendered frame
         *  \param      reason      The reason for changing frame. Usually, if emitted
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
        void                    lengthChanged( qint64 );
};

#endif // MAINWORKFLOW_H
