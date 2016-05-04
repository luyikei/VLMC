 /*****************************************************************************
 * ClipWorkflow.h : Clip workflow. Will extract a single frame from a VLCMedia
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

#ifndef CLIPWORKFLOW_H
#define CLIPWORKFLOW_H

#include "Tools/mdate.h"

#include "EffectsEngine/EffectUser.h"
#include "ClipHelper.h"
#include "Workflow/Types.h"
#include "Renderer/ClipSmemRenderer.h"

#include <QObject>
#include <QUuid>
#include <QXmlStreamWriter>

class   Clip;
class   Effect;
class   RendererEventWatcher;

class   QMutex;
class   QReadWriteLock;
class   QWaitCondition;

namespace Workflow
{
    class   Frame;
}

namespace Backend
{
    class   ISourceRenderer;
}

class   ClipWorkflow : public EffectUser
{
    Q_OBJECT

    public:
        enum        State
        {
            None = -1,
            /// \brief  Used when the clipworkflow hasn't been started yet
            Stopped,            //0
            /**
             *  \brief  Used when initializing is in progress (until the ISourceRenderer
             *          enters the playing state).
             */
            Initializing,       //1
            /// \brief  Used when the clipworkflow is launched and active
            Rendering,          //2
            /// \brief  Used when end is reached, IE no more frame has to be rendered, but the trackworkflow
            ///         may eventually ask for some.
            EndReached,         //3
            /// \brief  This state will be used when the ISourceRenderer is paused,
            ///         because of a sufficient number of computed buffers
            Paused,             //4
            /// \brief  An error was encountered, this ClipWorkflow must not be used anymore.
            Error               //5
        };

        ClipWorkflow( ClipHelper* clip );
        virtual ~ClipWorkflow();

        /**
         *  This method returns the current frame. It locks the renderMutex,
         *  therefore, you can call this method blindly, without taking care
         *  of the rendering process advancement.
         */
        Workflow::Frame*        getOutput( Workflow::TrackType trackType, ClipSmemRenderer::GetMode mode, qint64 currentFrame );
        /**
         * @brief Initialize base variables for the SourceRenderer.
         *
         * This may also perform some addditional initializations, and
         * therefore should be called before createSoutChain()
         */
        void                    initialize( quint32 width, quint32 height );

        /**
         *  \return             true if the ClipWorkflow is able to, and should render
         *                      a frame.
         *
         *  This is true when the state is not stopped, stopping, nor rendering.
         */
        bool                    shouldRender() const;

        /**
         *  Returns the current workflow state.
         */
        State                   getState() const;

        /**
            \brief              Returns the ClipHelper this workflow instance is based
                                uppon, so that you can query information on it.
            \return             A pointer to a ClipHelper instance.
        */
        inline ClipHelper*      getClipHelper()
        {
            return m_clipHelper;
        }
        inline Clip*            clip()
        {
            return m_clipHelper->clip();
        }

        /**
         *  \brief  Stop this workflow.
         */
        void                    stop();

        /**
         *  \brief  Set the rendering position
         *  \param  time    The position in millisecond
         *  \param  frame   The new current frame.
         */
        virtual void            setTime( qint64 time );

        bool                    waitForCompleteInit();

        /**
         *  \sa MainWorkflow::setFullSpeedRender();
         */
        void                    setFullSpeedRender( bool val );

        void                    mute();
        void                    unmute();
        bool                    isMuted() const;

        void                    requireResync();
        /**
         *  \return true if a resync is required.
         *
         *  If a resync is required, true will be returned, and the flag will be
         *  set back to false
         */
        bool                    isResyncRequired();

        virtual qint64          length() const;
        virtual Type            effectType() const;

    private:
        void                    adjustBegin();

    protected:
        void                    computePtsDiff( qint64 pts , Workflow::TrackType trackType );

        /**
         *  \brief  Will empty the computed buffers stack.
         *          This has to be implemented in the underlying
         *          clipworkflow implementation.
         */
        void                    flushComputedBuffers();


    private:
        /**
         * @brief m_initWaitCond Used to synchronize initialization.
         *
         * The associated lock is m_stateLock
         */
        QWaitCondition          *m_initWaitCond;
        /**
         *  \brief              Used by the trackworkflow to query a clipworkflow resync.
         *
         *  Basically, this will be used when a clip is moved, and therefore has to be
         *  updated.
         */
        bool                    m_resyncRequired;

    protected:
        ClipSmemRenderer*           m_renderer;
        RendererEventWatcher*       m_eventWatcher;
        ClipHelper*                 m_clipHelper;
        QMutex*                     m_renderLock;
        QReadWriteLock*             m_stateLock;
        State                       m_state;
        qint64                      m_previousPts[Workflow::NbTrackType];
        qint64                      m_currentPts[Workflow::NbTrackType];
        /**
         *  \brief  This is used for basic synchronisation when
         *          the clipworkflow hasn't generate a frame yet,
         *          while the renderer asks for one.
         */
        QWaitCondition          *m_renderWaitCond;
        qint64                  m_beginPausePts;
        qint64                  m_pauseDuration;
        bool                    m_fullSpeedRender;
        bool                    m_muted;

    private slots:
        void                    loadingComplete();
        void                    clipEndReached();
        void                    mediaPlayerPaused();
        void                    mediaPlayerUnpaused();
        void                    mediaPlayerStopped();
        void                    resyncClipWorkflow();

    protected slots:
        void                    errorEncountered();

    signals:
        void                    error( ClipWorkflow* );
        void                    bufferReachedMax();
};

#endif // CLIPWORKFLOW_H
