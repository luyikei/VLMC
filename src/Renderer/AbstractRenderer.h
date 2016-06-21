/*****************************************************************************
 * AbstractRenderer.h: Describe a common behavior for every renderers
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

#ifndef ABSTRACTRENDERER_H
#define ABSTRACTRENDERER_H

#include "config.h"
#include <memory>
#include <QObject>

#include "Workflow/Types.h"
#include "Tools/RendererEventWatcher.h"
#include "Backend/VLC/VLCSourceRenderer.h"
#include "Backend/IRenderTarget.h"

class   Clip;
class   Media;

/**
 *  \class  Common base for every renderer.
 */
class   AbstractRenderer : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY( AbstractRenderer )

protected:
    explicit AbstractRenderer();

public:
    virtual ~AbstractRenderer();

    /**
     *  \brief  Set the output volume.
     *  \param  volume the volume (int)
     *  \sa     getVolume()
     */
    virtual void        setVolume( int volume ) = 0;

    /**
     *  \brief          Play or pause the media.
     *
     *  This method is renderer dependant. It has to be implemented in the
     *  underlying renderer implementation.
     *  When this method is called :
     *      - if the render has not started the render is started
     *      - if the render has started and is not paused, the render will pause
     *      - if the render has started and is paused, the render will unpause
     *  \param  forcePause  Will force the pause if true.
     *  \sa     stop()
     */
    virtual void        togglePlayPause() = 0;

    /**
     *  \brief Render the next frame
     *  \sa     previousFrame()
     */
    virtual void                    nextFrame() = 0;

    /**
     *  \brief  Render the previous frame
     *  \sa     nextFrame();
     */
    virtual void                    previousFrame() = 0;

    /**
     *  \brief Stop the renderer.
     *  \sa togglePlayPause( bool );
     */
    virtual void                    stop() = 0;

    /**
     *  \brief   Return the volume
     *  \return  The Return the volume the audio level (int)
     *  \sa     setVolume( int )
     */
    virtual int                     getVolume() const = 0;

    /**
     * \brief   Return the length in milliseconds
     * \return  The length of the underlying rendered target in milliseconds
     *  \sa     getLength()
     */
    virtual qint64                  getLengthMs() const = 0;

    /**
     *  \brief  Return the current frame number
     *  \return The current frame
     */
    virtual qint64                  getCurrentFrame() const = 0;

    /**
     *  \brief Return the number of frames per second
     *  \return     The current fps
     */
    virtual float                   getFps() const = 0;

    /**
     *  \brief      Return the length in frames
     *  \warning    The returned value may not be accurate as it depends on FPS, that
     *              can be badly computed
     *  \return     The length that has to be rendered in frames
     *  \sa         getLengthMs()
     */
    virtual qint64                  length() const = 0;

    /**
     *  \brief  Return true if the renderer is paused
     *  \return true if the renderer is paused. false otherwise.
     */
    bool                            isPaused() const;

    /**
     *  \brief      Return true if the renderer is currently rendering.
     *  \return     true if the renderer is currently rendering. false otherwise.
     *              Note that a paused renderer is still rendering
     *  \sa         isPaused()
     */
    bool                            isRendering() const;

    void                            setRenderTarget(  std::unique_ptr<Backend::IRenderTarget> target );

    RendererEventWatcher*           eventWatcher();
protected:
    Backend::VLC::VLCSourceRenderer*       m_sourceRenderer;
    RendererEventWatcher*           m_eventWatcher;

    /**
     *  \brief  This flag allows us to know if the render is paused
     *          or not, without using libvlc, especially for the render preview.
     *  If the video is stopped, then this flag will be equal to false
     *  \warning    This is not thread safe.
     *  \sa         isPaused()
     */
    bool                            m_paused;
    bool                            m_isRendering;

    std::unique_ptr<Backend::IRenderTarget>         m_renderTarget;

public slots:
    /**
     *  \brief      This SLOT will be called when the time cursor has changed.
     *
     *  This mainly means that the current rendered frame should change.
     *  \param      newFrame    The new frame to render from.
     */
    virtual void                    previewWidgetCursorChanged( qint64 newFrame ) = 0;


signals:
    void                            frameChanged( qint64 newFrame,
                                                Vlmc::FrameChangedReason reason );
};

#endif // ABSTRACTRENDERER_H
