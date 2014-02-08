/*****************************************************************************
 * AudioClipWorkflow.cpp : Clip workflow. Will extract a single frame from a VLCMedia
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

#include "AudioClipWorkflow.h"

#include "VLCMedia.h"
#include "VlmcDebug.h"
#include "Workflow/Types.h"

#include <QMutexLocker>
#include <QStringBuilder>

AudioClipWorkflow::AudioClipWorkflow( ClipHelper *ch ) :
        ClipWorkflow( ch ),
        m_lastReturnedBuffer( NULL )
{
    m_ptsOffset = 0;
}

AudioClipWorkflow::~AudioClipWorkflow()
{
    stop();
}

void
AudioClipWorkflow::preallocate()
{
    for ( quint32 i = 0; i < AudioClipWorkflow::nbBuffers; ++i )
    {
        Workflow::AudioSample *as = new Workflow::AudioSample;
        as->buff = NULL;
        m_availableBuffers.push_back( as );
    }
}

void
AudioClipWorkflow::releasePrealocated()
{
    while ( m_availableBuffers.isEmpty() == false )
    {
        Workflow::AudioSample *as = m_availableBuffers.takeFirst();
        delete as->buff;
        delete as;
    }
    while ( m_computedBuffers.isEmpty() == false )
    {
        Workflow::AudioSample *as = m_computedBuffers.takeFirst();
        delete as->buff;
        delete as;
    }
}

void*
AudioClipWorkflow::getLockCallback() const
{
    return reinterpret_cast<void*>( &AudioClipWorkflow::lock );
}

void*
AudioClipWorkflow::getUnlockCallback() const
{
    return reinterpret_cast<void*>( &AudioClipWorkflow::unlock );
}

Workflow::OutputBuffer*
AudioClipWorkflow::getOutput( ClipWorkflow::GetMode mode, qint64 )
{
    QMutexLocker    lock( m_renderLock );

    if ( m_lastReturnedBuffer != NULL )
    {
        m_availableBuffers.enqueue( m_lastReturnedBuffer );
        m_lastReturnedBuffer = NULL;
    }
    if ( getNbComputedBuffers() == 0 )
        return NULL;
    if ( shouldRender() == false )
        return NULL;
    if ( mode == ClipWorkflow::Get )
        vlmcCritical() << "A sound buffer should never be asked with 'Get' mode";
    Workflow::AudioSample   *buff = m_computedBuffers.dequeue();
    if ( m_previousPts == -1 )
    {
        buff->ptsDiff = 0;
        m_previousPts = buff->pts;
    }
    else
    {
        buff->ptsDiff = buff->pts - m_previousPts;
        m_previousPts = buff->pts;
    }
    postGetOutput();
    m_lastReturnedBuffer = buff;
    return buff;
}

QString
AudioClipWorkflow::createSoutChain() const
{
    QString chain = ":sout=#transcode{acodec=f32l,"
                    "samplerate=48000,channels=2,no-hurry-up}:smem{";
    if ( m_fullSpeedRender == false )
        chain += "time-sync";
    else
        chain += "no-time-sync";
    chain += ",audio-data=" % QString::number( reinterpret_cast<intptr_t>( this ) )
            % ",audio-prerender-callback="
            % QString::number( reinterpret_cast<intptr_t>( getLockCallback() ) )
            % ",audio-postrender-callback="
            % QString::number( reinterpret_cast<intptr_t>( getUnlockCallback() ) )
            % '}';
    return chain;
}

void AudioClipWorkflow::initializeVlcOutput()
{
    preallocate();
    m_vlcMedia->addOption(":no-sout-video");
    m_vlcMedia->addOption(":no-video");
}

Workflow::AudioSample*
AudioClipWorkflow::createBuffer( size_t size )
{
    Workflow::AudioSample *as = new Workflow::AudioSample;
    as->buff = new uchar[size];
    as->size = size;
    return as;
}

void
AudioClipWorkflow::lock( AudioClipWorkflow *cw, quint8 **pcm_buffer , size_t size )
{
    cw->m_renderLock->lock();

    Workflow::AudioSample     *as = NULL;
    if ( cw->m_availableBuffers.isEmpty() == true )
        as = cw->createBuffer( size );
    else
    {
        as = cw->m_availableBuffers.dequeue();
        if ( as->buff == NULL )
        {
            as->buff = new uchar[size];
            as->size = size;
        }
    }
    cw->m_computedBuffers.enqueue( as );
    *pcm_buffer = as->buff;
}

void
AudioClipWorkflow::unlock( AudioClipWorkflow *cw, quint8 *pcm_buffer,
                                      quint32 channels, quint32 rate,
                                      quint32 nb_samples, quint32 bits_per_sample,
                                      size_t size, qint64 pts )
{
    Q_UNUSED( pcm_buffer );
    Q_UNUSED( rate );
    Q_UNUSED( bits_per_sample );
    Q_UNUSED( size );

    pts -= cw->m_ptsOffset;
    Workflow::AudioSample* as = cw->m_computedBuffers.last();
    if ( as->buff != NULL )
    {
        as->nbSample = nb_samples;
        as->nbChannels = channels;
        as->ptsDiff = 0;
        as->pts = pts;
        if ( cw->m_pauseDuration != -1 )
        {
            cw->m_ptsOffset += cw->m_pauseDuration;
            cw->m_pauseDuration = -1;
        }
        if ( cw->m_currentPts > pts )
        {
            cw->m_computedBuffers.removeLast();
            cw->insertPastBlock( as );
        }
        else
            cw->m_currentPts = pts;
    }
    cw->commonUnlock();
    cw->m_renderLock->unlock();
}

void
AudioClipWorkflow::insertPastBlock( Workflow::AudioSample *as )
{
    QQueue<Workflow::AudioSample*>::iterator    it = m_computedBuffers.begin();
    QQueue<Workflow::AudioSample*>::iterator    end = m_computedBuffers.end();

    while ( it != end )
    {
        if ( (*it)->pts > as->pts )
        {
            m_computedBuffers.insert( it, as );
            return ;
        }
        ++it;
    }
    //Fail safe: reinsert the block at the end.
    m_computedBuffers.push_back( as );
}

quint32
AudioClipWorkflow::getNbComputedBuffers() const
{
    return m_computedBuffers.count();
}

quint32
AudioClipWorkflow::getMaxComputedBuffers() const
{
    return AudioClipWorkflow::nbBuffers;
}

void
AudioClipWorkflow::flushComputedBuffers()
{
    QMutexLocker        lock( m_renderLock );

    while ( m_computedBuffers.isEmpty() == false )
    {
        m_availableBuffers.enqueue( m_computedBuffers.dequeue() );
    }
}
