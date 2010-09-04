/*****************************************************************************
 * VideoClipWorkflow.h : Clip workflow. Will extract a single frame from a VLCMedia
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

#ifndef VIDEOCLIPWORKFLOW_H
#define VIDEOCLIPWORKFLOW_H

#include "ClipWorkflow.h"
#include "EffectsEngine.h"

#include <QQueue>

class   Clip;

class   VideoClipWorkflow : public ClipWorkflow
{
    Q_OBJECT

    public:
        VideoClipWorkflow( ClipHelper* ch );
        ~VideoClipWorkflow();
        void                    *getLockCallback() const;
        void                    *getUnlockCallback() const;
        virtual Workflow::OutputBuffer  *getOutput( ClipWorkflow::GetMode mode );
        virtual void            setTime( qint64 time, qint64 frame );

        static const quint32    nbBuffers = 3 * 30; //3 seconds with an average fps of 30

    protected:
        virtual void            initVlcOutput();
        virtual quint32         getNbComputedBuffers() const;
        virtual quint32         getMaxComputedBuffers() const;
        virtual void            flushComputedBuffers();
        /**
         *  \brief              Pre-allocate some image buffers.
         */
        void                    preallocate();
        virtual void            releasePrealocated();

    private:
        QQueue<Workflow::Frame*>    m_computedBuffers;
        QQueue<Workflow::Frame*>    m_availableBuffers;
        static void                 lock( VideoClipWorkflow* clipWorkflow, void** pp_ret,
                                            int size );
        static void                 unlock( VideoClipWorkflow* clipWorkflow, void* buffer,
                                            int width, int height, int bpp, int size,
                                            qint64 pts );
        QMutex                      *m_renderedFrameMutex;
        qint64                      m_renderedFrame;
        Workflow::Frame             *m_lastReturnedBuffer;
};

#endif // VIDEOCLIPWORKFLOW_H
