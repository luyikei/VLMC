/*****************************************************************************
 * AudioClipWorkflow.h : Clip workflow. Will extract a single frame from a VLCMedia
 *****************************************************************************
 * Copyright (C) 2008-2010 VideoLAN
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

#ifndef AUDIOCLIPWORKFLOW_H
#define AUDIOCLIPWORKFLOW_H

#include "ClipWorkflow.h"

#include <QPointer>
#include <QQueue>

namespace Workflow
{
    class  AudioSample;
}

class   AudioClipWorkflow : public ClipWorkflow
{
    Q_OBJECT

    public:
        AudioClipWorkflow( ClipHelper* ch );
        ~AudioClipWorkflow();
        void                        *getLockCallback() const;
        void                        *getUnlockCallback() const;
        virtual Workflow::OutputBuffer  *getOutput( ClipWorkflow::GetMode mode, qint64 currentFrame );
    protected:
        virtual quint32             getNbComputedBuffers() const;
        virtual quint32             getMaxComputedBuffers() const;
        virtual void                flushComputedBuffers();
        void                        preallocate();
        virtual void                releasePrealocated();

    private:
        virtual QString             createSoutChain() const;
        virtual void                initializeVlcOutput();
        Workflow::AudioSample*      createBuffer( size_t size );
        void                        insertPastBlock( Workflow::AudioSample* as );
        static void                 lock(AudioClipWorkflow* clipWorkflow,
                                          quint8** pcm_buffer , size_t size );
        static void                 unlock(AudioClipWorkflow* clipWorkflow,
                                            quint8* pcm_buffer, quint32 channels,
                                            quint32 rate, quint32 nb_samples,
                                            quint32 bits_per_sample,
                                            size_t size, qint64 pts );

    private:
        QQueue<Workflow::AudioSample*>      m_computedBuffers;
        QQueue<Workflow::AudioSample*>      m_availableBuffers;
        qint64                              m_ptsOffset;
        Workflow::AudioSample               *m_lastReturnedBuffer;
        static const quint32   nbBuffers = 256;
};

#endif // AUDIOCLIPWORKFLOW_H
