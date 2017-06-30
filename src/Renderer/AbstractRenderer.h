/*****************************************************************************
 * AbstractRenderer.h: Describe a common behavior for every renderers
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

#ifndef ABSTRACTRENDERER_H
#define ABSTRACTRENDERER_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <memory>
#include <QObject>
#include <QSharedPointer>

#include "Workflow/Types.h"
#include "Backend/IOutput.h"

class   Clip;
class   Media;

class RendererEventWatcher;

namespace Backend
{
class IOutput;
class IInput;
}

/**
 *  \class  Common base for every renderer.
 */
class   AbstractRenderer : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY( AbstractRenderer )

public:
    explicit AbstractRenderer();
    virtual ~AbstractRenderer();

    /**
     *  \brief  Set the output volume.
     *  \param  volume the volume (int)
     *  \sa     getVolume()
     */
    virtual void        setVolume( int volume );

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
    virtual void        togglePlayPause();

    /**
     *  \brief Render the next frame
     *  \sa     previousFrame()
     */
    virtual void                    nextFrame();

    /**
     *  \brief  Render the previous frame
     *  \sa     nextFrame();
     */
    virtual void                    previousFrame();

    /**
     *  \brief Stop the renderer.
     *  \sa togglePlayPause( bool );
     */
    virtual void                    stop();

    virtual void                    setPosition( qint64 pos );

    /**
     *  \brief   Return the volume
     *  \return  The Return the volume the audio level (int)
     *  \sa     setVolume( int )
     */
    virtual int                     getVolume() const;

    /**
     * \brief   Return the length in milliseconds
     * \return  The length of the underlying rendered target in milliseconds
     *  \sa     getLength()
     */
    virtual qint64                  getLengthMs() const;

    /**
     *  \brief  Return the current frame number
     *  \return The current frame
     */
    virtual qint64                  getCurrentFrame() const;

    /**
     *  \brief Return the number of frames per second
     *  \return     The current fps
     */
    virtual float                   getFps() const;

    /**
     *  \brief      Return the length in frames
     *  \warning    The returned value may not be accurate as it depends on FPS, that
     *              can be badly computed
     *  \return     The length that has to be rendered in frames
     *  \sa         getLengthMs()
     */
    virtual qint64                  length() const;

    /**
     *  \brief  Return true if the renderer is paused
     *  \return true if the renderer is paused. false otherwise.
     */
    virtual bool                    isPaused() const;

    /**
     *  \brief      Return true if the renderer is currently rendering.
     *  \return     true if the renderer is currently rendering. false otherwise.
     *              Note that a paused renderer is still rendering
     *  \sa         isPaused()
     */
    virtual bool                    isRendering() const;

    virtual void                    setInput( Backend::IInput* input );
    virtual void                    setOutput( std::unique_ptr<Backend::IOutput> consuemr );

    QSharedPointer<RendererEventWatcher>           eventWatcher();
protected:
    std::unique_ptr<Backend::IOutput>             m_output;

    Backend::IInput*                             m_input;
    QSharedPointer<RendererEventWatcher>           m_eventWatcher;


public slots:
    /**
     *  \brief      This SLOT will be called when the time cursor has changed.
     *
     *  This mainly means that the current rendered frame should change.
     *  \param      newFrame    The new frame to render from.
     */
    virtual void                    previewWidgetCursorChanged( qint64 newFrame );


signals:
    void                            frameChanged( qint64 newFrame,
                                                Vlmc::FrameChangedReason reason );
    void                            lengthChanged( qint64 length ); // In frames
};

#endif // ABSTRACTRENDERER_H
