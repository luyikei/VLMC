/*****************************************************************************
 * WorkflowRenderer.h: Render the main workflow
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

#ifndef WORKFLOWRENDERER_H
#define WORKFLOWRENDERER_H

#include "GenericRenderer.h"
#include "Backend/ISourceRenderer.h"
#include "Workflow/MainWorkflow.h"

#include <QObject>
#include <QTime>

namespace Backend
{
    class IBackend;
    class IMemorySource;
}

class   Clip;

class   QWidget;
class   QWaitCondition;
class   QMutex;

class   WorkflowRenderer : public GenericRenderer
{
    Q_OBJECT
    Q_DISABLE_COPY( WorkflowRenderer )

    public:
        /**
         *  \brief  This struct will be the type of the callback parameter
         *          in the lock / unlock callbacks
         */
        struct  EsHandler
        {
            WorkflowRenderer*   self; ///< The 'this' pointer will be passed in this field
            //Fixme: this should go away I guess...
            double              fps; ///< The fps to use for this rendering session.
        };

        WorkflowRenderer( Backend::IBackend *backend, MainWorkflow* workflow );
        ~WorkflowRenderer();

        /**
         *  \brief          Play or pause the media.
         *
         *  When this method is called :
         *      - if the render has not started and forcePause is false, the render is started
         *      - if the render has not started and forcePause is true, nothing happens.
         *      - if the render has started and is not paused, the render will pause
         *      - if the render has started, and is paused, the render will unpause if
         *          forcePause is false.
         *  \param  forcePause  Will force the pause if true.
         *  \sa     stop()
         *  \warning    Do NOT call this method from a constructor as it calls some
         *              virtual methods.
         */
        virtual void        togglePlayPause();
        /**
         *  \brief Stop the mainworkflow, but not the renderer.
         *
         *  In order to provide premanent feedback (ie stay in paused mode when not
         *  playing, we have to never stop the renderer.
         *  \sa togglePlayPause( bool );
         *  \sa killRenderer();
         */
        virtual void        stop();

        /**
         *  \brief  Set the output volume.
         *  \param  volume the volume (int)
         *  \return 0 if the volume was set, -1 if it was out of range
         *  \sa     getVolume()
         */
        virtual void        setVolume( int volume );

        /**
         *  \brief   Return the volume
         *  \return  The Return the volume the audio level (int)
         *  \sa     setVolume( int )
         */
        virtual int         getVolume() const;

        /**
         *  \brief Render the next frame
         *  \sa     previousFrame()
         */
        virtual void        nextFrame();
        /**
         *  \brief  Render the previous frame
         *  \sa     nextFrame();
         */
        virtual void        previousFrame();
        /**
         * \brief   Return the length in milliseconds
         * \return  The length of the underlying rendered target in milliseconds
         *  \sa     getLength()
         */
        virtual qint64      getLengthMs() const;

        virtual qint64      length() const;

        /**
         *  \brief  Return the current frame number
         *  \return The current frame
         */
        virtual qint64      getCurrentFrame() const;
        /**
         *  \brief Return the number of frames per second
         *  \return     The current fps
         */
        virtual float       getFps() const;

        void                startRenderToFile( const QString& outputFileName, quint32 width, quint32 height,
                                               double fps, const QString& ar,
                                               quint32 vbitrate, quint32 abitrate );

    private:
        void                start();
        /**
         *  \brief          This is a subpart of the togglePlayPause( bool ) method
         *
         *  It starts the render by launching the media player that will query the
         *  lock and unlock callbacks, thus querying the MainWorkflow for frames and
         *  audio samples
         *  \sa             togglePlayPause( bool );
         */
        virtual void        startPreview();

    protected:
        /**
         *  \brief          Will return a pointer to the function/static method to use
         *                  as the imem lock callback.
         *  This method is provided to allow renderers to inherit from this class,
         *  without having to reimplement the initializeRenderer() method, that uses
         *  both getLockCallback() and getUnlockCallback()
         *  \return         A pointer to the lock function.
         *  \sa             getUnlockCallback()
         */
        virtual Backend::ISourceRenderer::MemoryInputLockCallback getLockCallback();
        /**
         *  \brief          Will return a pointer to the function/static method to use
         *                  as the imem unlock callback.
         *  This method is provided to allow renderers to inherit from this class,
         *  without having to reimplement the initializeRenderer() method, that uses
         *  both getLockCallback() and getUnlockCallback()
         *  \return         A pointer to the unlock function.
         *  \sa             getLockCallback()
         */
        virtual Backend::ISourceRenderer::MemoryInputUnlockCallback getUnlockCallback();
        /**
         *  \brief              Lock callback for imem module
         *
         *  This callback will query the MainWorkflow for a frame or an audio sample
         *  \param  data        The callback data, this is most likely to be an EsHandler
         *  \param  cookie      The input identifier.
         *  \param  dts         Unused, but provided by imem
         *  \param  pts         The pts for the buffer that will be provided
         *  \param  flags       Unused but provided by imem
         *  \param  bufferSize  The size of the buffer that will be provided
         *  \param  buffer      The buffer itself.
         */
        static int          lock(void *data, const char* cookie, int64_t *dts, int64_t *pts,
                                unsigned int *flags, size_t *bufferSize, const void **buffer );
        /**
         *  \brief  "Subcallback", for video frame injection
         *
         *  \param  pts         The pts for the buffer that will be provided
         *  \param  bufferSize  The size of the buffer that will be provided
         *  \param  buffer      The buffer itself.
         */
        int                 lockVideo(void *handler, int64_t *pts,
                                       size_t *bufferSize, const void **buffer );
        /**
         *  \brief  "Subcallback", for audio sample injection
         *
         *  \param  pts         The pts for the buffer that will be provided
         *  \param  bufferSize  The size of the buffer that will be provided
         *  \param  buffer      The buffer itself.
         */
        int                 lockAudio( EsHandler *handler, int64_t *pts,
                                       size_t *bufferSize, const void **buffer );
        /**
         *  \brief  unlock callback for the imem module
         *
         *  \param  data        The callback data, this is most likely to be an EsHandler
         *  \param  cookie      The imem cookie.
         *  \param  buffSize    The size of the buffer
         *  \param  buffer      The buffer to be released
         */
        static void         unlock( void *data, const char* cookie, size_t buffSize, void *buffer );

        /**
         *  \brief          Configure the production chain.
         */
        void                setupRenderer();

    protected:
        MainWorkflow*       m_mainWorkflow;
        Backend::IMemorySource* m_source;
        bool                m_stopping;
        float               m_outputFps;
        QString             m_aspectRatio;
        /**
         *  \brief          This isn't exactly the current PTS.
         *                  It's the number of frame rendered since the render has started.
         */
        qint64              m_pts;
        qint64              m_audioPts;

    private:

        quint32             m_width;
        quint32             m_height;

        /**
         *  \brief          When there's no sound to play, this is the buffer that'll
         *                  be injected
         */
        quint8              *m_silencedAudioBuffer;
        EsHandler*          m_esHandler;
        quint32             m_nbChannels;
        quint32             m_rate;
        /**
         *  \brief          Used in permanent rendering mode, to know if some operations
         *                  has to be performed.
         */
        qint64              m_oldLength;

        static const quint8     VideoCookie = '0';
        static const quint8     AudioCookie = '1';

        // For Preview
        QTime                       m_time;

    public slots:
        /**
         *  \brief          The current frame just changed because of the timeline cursor
         */
        void                timelineCursorChanged( qint64 newFrame );
        /**
         *  \brief          The current frame just changed because of the timeline ruler
         */
        void                rulerCursorChanged( qint64 newFrame );
        /**
         *  \brief          The current frame just changed because of the preview widget
         */
        void                previewWidgetCursorChanged( qint64 newFrame );

    private slots:
        /**
         *  \brief          Used to launch "permanent playback", as soon as the length
         *                  first changed to a non zero value.
         *
         *  If the length comes to a 0 value again, the permanent playback will be stoped.
         */
        void                mainWorkflowLenghtChanged( qint64 newLength );

    signals:
        void                        imageUpdated( const uchar* image );
        void                        renderComplete();
};

#endif // WORKFLOWRENDERER_H
