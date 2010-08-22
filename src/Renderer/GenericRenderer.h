/*****************************************************************************
 * GenericRenderer.h: Describe a common behavior for every renderers
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

#ifndef GENERICRENDERER_H
#define GENERICRENDERER_H

#include "config.h"

#include <QObject>
#ifdef WITH_GUI
# include <QWidget>
#endif

#include "Types.h"

class   Clip;
class   Media;
class   QUuid;
namespace LibVLCpp
{
    class   MediaPlayer;
}

/**
 *  \class  Common base for every renderer.
 */
class   GenericRenderer : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY( GenericRenderer );

public:
    explicit GenericRenderer();
    virtual ~GenericRenderer();

#ifdef WITH_GUI
    /**
     *  \brief  Set the widget used for rendering the output.
     *  \param  renderWidget    The widget to use for render.
     *  \sa     setPreviewLabel( QLabel* )
     */
    void                setRenderWidget( QWidget* renderWidget );
#endif
    /**
     *  \brief          Play or pause the media.
     *
     *  This method is renderer dependant. It has to be implemented in the
     *  underlying renderer implementation.
     *  When this method is called :
     *      - if the render has not started and forcePause is false, the render is started
     *      - if the render has not started and forcePause is true, nothing happens.
     *      - if the render has started and is not paused, the render will pause
     *      - if the render has started, and is paused, the render will unpause if
     *          forcePause is false.
     *  \param  forcePause  Will force the pause if true.
     *  \sa     stop()
     */
    virtual void        togglePlayPause( bool forcePause = false ) = 0;

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
    virtual qint64                  getLength() const = 0;

    /**
     *  \brief  Return true if the renderer is paused
     *  \return true if the renderer is paused. false otherwise.
     */
    bool                            isPaused() const
    {
        return m_paused;
    }

    /**
     *  \brief      Return true if the renderer is currently rendering.
     *  \return     true if the renderer is currently rendering. false otherwise.
     *              Note that a paused renderer is still rendering
     *  \sa         isPaused()
     */
    bool                            isRendering() const
    {
        return m_isRendering;
    }

protected:
    /**
     *  \brief  The media player that will be used for rendering
     */
    LibVLCpp::MediaPlayer*          m_mediaPlayer;
    /**
     *  \brief  This flag allows us to know if the render is paused
     *          or not, without using libvlc, especially for the render preview.
     *  If the video is stopped, then this flag will be equal to false
     *  \warning    This is not thread safe.
     *  \sa         isPaused()
     */
    bool                            m_paused;

    /**
     *  \brief  Will be equal to true if a render has been started, even if it paused.
     *  \sa     isRendering()
     */
    bool                            m_isRendering;

    /**
     *  \brief      The QWidget on which we will render.
     *  \sa         setRenderWidget( QWidget* );
     */
    QWidget*                        m_renderWidget;

public slots:
    /**
     *  \brief      This SLOT has to be called when the render ends.
     */
    virtual void                    __endReached() = 0;
    /**
     *  \brief      This SLOT will be called when the time cursor has changed.
     *
     *  This mainly means that the current rendered frame should change.
     *  \param      newFrame    The new frame to render from.
     */
    virtual void                    previewWidgetCursorChanged( qint64 newFrame ) = 0;


signals:
    /**
     *  \brief  This signal means the render just finished, and has been stoped.
     *  \sa     endReached()
     */
    void                            stopped();
    /**
     *  \brief  Emmited when the render has been paused.
     *  \sa     playing()
     */
    void                            paused();
    /**
     *  \brief  Emmited when the renderer has started to render, and has been unpaused.
     *  \sa     paused()
     */
    void                            playing();
    /**
     *  \brief  Emmited when rendered frame has been changed.
     *  \param  newFrame    The new current frame
     *  \param  reason      The reason for changing frame
     */
    void                            frameChanged( qint64 newFrame,
                                                Vlmc::FrameChangedReason reason );
    /**
     *  \brief  Emmited when render end is reached.
     *
     *  This should be emmited just before stopped
     *  \sa     stopped();
     */
    void                            endReached();

    /**
     *  \brief  Emited when something went wrong with the render.
     *
     *  The cause may vary depending on the underlying renderer, though this will
     *  almost always be caused by a missing codec.
     */
    void                error();

};

#endif // GENERICRENDERER_H
