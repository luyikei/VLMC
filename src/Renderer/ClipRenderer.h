/*****************************************************************************
 * ClipRenderer.h: Preview widget
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

#ifndef CLIPRENDERER_H
#define CLIPRENDERER_H

#include "AbstractRenderer.h"

#include <QObject>

class   Clip;
class   Media;

class ClipRenderer : public AbstractRenderer
{
    Q_OBJECT
    Q_DISABLE_COPY( ClipRenderer )

public:
    explicit ClipRenderer();
    virtual ~ClipRenderer();

    virtual void            togglePlayPause();
    virtual void            stop();
    virtual void            setVolume( int volume );
    virtual int             getVolume() const;
    virtual void            nextFrame();
    virtual void            previousFrame();
    virtual qint64          length() const;
    virtual qint64          getLengthMs() const;
    virtual qint64          getCurrentFrame() const;
    virtual float           getFps() const;
    virtual Clip            *getClip();

private:
    void                    startPreview();

private:
    bool                    m_clipLoaded;
    Clip*                   m_selectedClip;
    qint64                  m_begin;
    qint64                  m_end;
    /**
     *  \brief  This flags is used to know if a new media has been selected in the
     * library. If so, we must relaunch the render if the play button is clicked again.
     */
    bool                    m_mediaChanged;

public slots:
    /**
     *  \brief      Set the Clip to render
     *  \param      clip    The clip to render
     */
    void                    setClip( Clip* clip );
    void                    clipUnloaded( const QUuid& uuid );
    virtual void            previewWidgetCursorChanged( qint64 newFrame );
    void                    updateInfos( Clip* clip );

    /**
     *  \brief      Triggered at every libvlc_MediaPlayerTimeChanged event.
     *
     *  This slot will compute a frame number based on the time and the clip's FPS.
     *  Once computed, it will emit a frameChanged signal, with the reason Renderer.
     *  \warning    The frame number computed may be unaccurate.
     *  \sa         frameChanged();
     */
    void                    timeChanged( qint64 time );
    void                    videoStopped();
    void                    videoPaused();
    void                    videoPlaying();
};

#endif // CLIPRENDERER_H
