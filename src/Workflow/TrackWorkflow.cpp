/*****************************************************************************
 * TrackWorkflow.cpp : Will query the Clip workflow for each successive clip in the track
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


#include "TrackWorkflow.h"

#include "Clip.h"
#include "ClipHelper.h"
#include "AudioClipWorkflow.h"
#include "ImageClipWorkflow.h"
#include "Media.h"
#include "MixerInstance.h"
#include "Types.h"
#include "VideoClipWorkflow.h"
#include "vlmc.h"

#include <QReadWriteLock>
#include <QDomDocument>
#include <QDomElement>

#include <QtDebug>

TrackWorkflow::TrackWorkflow( MainWorkflow::TrackType type  ) :
        m_length( 0 ),
        m_trackType( type ),
        m_lastFrame( 0 ),
        m_videoStackedBuffer( NULL ),
        m_audioStackedBuffer( NULL )
{
    m_renderOneFrameMutex = new QMutex;
    m_clipsLock = new QReadWriteLock;
    m_mixerBuffer = new Workflow::Frame;
}

TrackWorkflow::~TrackWorkflow()
{
    QMap<qint64, ClipWorkflow*>::iterator       it = m_clips.begin();
    QMap<qint64, ClipWorkflow*>::iterator       end = m_clips.end();

    while ( it != end )
    {
        stopClipWorkflow( it.value() );
        delete it.value();
        it = m_clips.erase( it );
    }
    delete m_clipsLock;
    delete m_renderOneFrameMutex;
}

void
TrackWorkflow::addClip( ClipHelper* ch, qint64 start )
{
    ClipWorkflow* cw;
    if ( m_trackType == MainWorkflow::VideoTrack )
    {
        if ( ch->clip()->getMedia()->fileType() == Media::Video )
            cw = new VideoClipWorkflow( ch );
        else
            cw = new ImageClipWorkflow( ch );
    }
    else
        cw = new AudioClipWorkflow( ch );
    addClip( cw, start );
}

void
TrackWorkflow::addClip( ClipWorkflow* cw, qint64 start )
{
    QWriteLocker    lock( m_clipsLock );
    m_clips.insert( start, cw );
    computeLength();
}

void
TrackWorkflow::addEffect( Effect *effect, const QUuid &uuid )
{
    QMap<qint64, ClipWorkflow*>::const_iterator     it = m_clips.begin();
    QMap<qint64, ClipWorkflow*>::const_iterator     end = m_clips.end();

    while ( it != end )
    {
        if ( it.value()->getClipHelper()->clip()->fullId() == uuid )
            it.value()->appendEffect( effect );
        ++it;
    }
}

//Must be called from a thread safe method (m_clipsLock locked)
void
TrackWorkflow::computeLength()
{
    if ( m_clips.count() == 0 )
    {
        m_length = 0;
        return ;
    }
    QMap<qint64, ClipWorkflow*>::const_iterator it = m_clips.end() - 1;
    m_length = (it.key() + it.value()->getClipHelper()->length() );
}

qint64
TrackWorkflow::getLength() const
{
    return m_length;
}

qint64
TrackWorkflow::getClipPosition( const QUuid& uuid ) const
{
    QMap<qint64, ClipWorkflow*>::const_iterator     it = m_clips.begin();
    QMap<qint64, ClipWorkflow*>::const_iterator     end = m_clips.end();

    while ( it != end )
    {
        if ( it.value()->getClipHelper()->uuid() == uuid )
            return it.key();
        ++it;
    }
    return -1;
}

Clip*
TrackWorkflow::getClip( const QUuid& uuid )
{
    QMap<qint64, ClipWorkflow*>::const_iterator     it = m_clips.begin();
    QMap<qint64, ClipWorkflow*>::const_iterator     end = m_clips.end();

    while ( it != end )
    {
        if ( it.value()->getClipHelper()->clip()->uuid() == uuid )
            return it.value()->clip();
        ++it;
    }
    return NULL;
}

void*
TrackWorkflow::renderClip( ClipWorkflow* cw, qint64 currentFrame,
                                        qint64 start , bool needRepositioning,
                                        bool renderOneFrame, bool paused )
{
    ClipWorkflow::GetMode       mode = ( paused == false || renderOneFrame == true ?
                                         ClipWorkflow::Pop : ClipWorkflow::Get );

    cw->getStateLock()->lockForRead();
    if ( cw->getState() == ClipWorkflow::Rendering ||
         cw->getState() == ClipWorkflow::Paused ||
         cw->getState() == ClipWorkflow::PauseRequired ||
         cw->getState() == ClipWorkflow::UnpauseRequired )
    {
        cw->getStateLock()->unlock();

        if ( cw->isResyncRequired() == true || needRepositioning == true )
            adjustClipTime( currentFrame, start, cw );
        return cw->getOutput( mode );
    }
    else if ( cw->getState() == ClipWorkflow::Stopped )
    {
        cw->getStateLock()->unlock();
        cw->initialize();
        //If the init failed, don't even try to call getOutput.
        if ( cw->waitForCompleteInit() == false )
            return NULL;
        //We check for a difference greater than one to avoid false positive when starting.
        if ( (  qAbs(start - currentFrame) > 1 ) || cw->getClipHelper()->begin() != 0 )
        {
            //Clip was not started at its real begining: adjust the position
            adjustClipTime( currentFrame, start, cw );
        }
        return cw->getOutput( mode );
    }
    else if ( cw->getState() == ClipWorkflow::EndReached ||
              cw->getState() == ClipWorkflow::Muted ||
              cw->getState() == ClipWorkflow::Error )
    {
        cw->getStateLock()->unlock();
        //The stopClipWorkflow() method will take care of that.
    }
    else
    {
        qCritical() << "Unexpected state:" << cw->getState();
        cw->getStateLock()->unlock();
    }
    return NULL;
}

void
TrackWorkflow::preloadClip( ClipWorkflow* cw )
{
    cw->getStateLock()->lockForRead();

    if ( cw->getState() == ClipWorkflow::Stopped )
    {
        cw->getStateLock()->unlock();
        cw->initialize();
        return ;
    }
    cw->getStateLock()->unlock();
}

void
TrackWorkflow::stopClipWorkflow( ClipWorkflow* cw )
{
//    qDebug() << "Stopping clip workflow";
    cw->getStateLock()->lockForRead();

    if ( cw->getState() == ClipWorkflow::Stopped ||
         cw->getState() == ClipWorkflow::Muted ||
         cw->getState() == ClipWorkflow::Error )
    {
        cw->getStateLock()->unlock();
        return ;
    }
    cw->getStateLock()->unlock();
    cw->stop();
}

bool
TrackWorkflow::hasFrameToRender( qint64 currentFrame ) const
{
    if ( m_clips.size() == 0 )
        return true;
    //This is the last video by chronological order :
    QMap<qint64, ClipWorkflow*>::const_iterator   it = m_clips.end() - 1;
    ClipWorkflow* cw = it.value();
    //Check if the Clip is in error state. If so, don't bother checking anything else.
    {
        QReadLocker     lock( cw->getStateLock() );
        if ( cw->getState() == ClipWorkflow::Error )
            return true;
    }
    //If it ends before the current frame, we reached end.
    return ( cw->getClipHelper()->length() + it.key() < currentFrame );
}

void
TrackWorkflow::stop()
{
    QMap<qint64, ClipWorkflow*>::iterator       it = m_clips.begin();
    QMap<qint64, ClipWorkflow*>::iterator       end = m_clips.end();

    while ( it != end )
    {
        stopClipWorkflow( it.value() );
        ++it;
    }
    releasePreviousRender();
    m_lastFrame = 0;
}

void
TrackWorkflow::releasePreviousRender()
{
    if ( m_audioStackedBuffer != NULL )
    {
        m_audioStackedBuffer->release();
        m_audioStackedBuffer = NULL;
    }
    if ( m_videoStackedBuffer != NULL )
    {
        m_videoStackedBuffer->release();
        m_videoStackedBuffer = NULL;
    }
}

void*
TrackWorkflow::getOutput( qint64 currentFrame, qint64 subFrame, bool paused )
{
    releasePreviousRender();
    QReadLocker     lock( m_clipsLock );

    QMap<qint64, ClipWorkflow*>::iterator       it = m_clips.begin();
    QMap<qint64, ClipWorkflow*>::iterator       end = m_clips.end();
    bool                                        needRepositioning;
    void                                        *ret = NULL;
    StackedBuffer<Workflow::Frame*>             *frames[EffectsEngine::MaxFramesForMixer];
    quint32                                     frameId = 0;
    bool                                        renderOneFrame = false;

    if ( m_lastFrame == -1 )
        m_lastFrame = currentFrame;
    {
        QMutexLocker      lock2( m_renderOneFrameMutex );
        if ( m_renderOneFrame == true )
        {
            m_renderOneFrame = false;
            renderOneFrame = true;
        }
    }
    {
        // This is a bit hackish : when we want to pop a frame in renderOneFrame mode,
        // we also set the position to avoid the stream to be missynchronized.
        // this frame setting will most likely toggle the next condition as true
        // If this condition is true, the clipworkflow will flush all its buffer
        // as we need to resynchronize after a setTime, so this condition has to remain
        // false. Easy ain't it?
        if ( paused == true && subFrame != m_lastFrame && renderOneFrame == false)
            needRepositioning = true;
        else
            needRepositioning = ( abs( subFrame - m_lastFrame ) > 1 ) ? true : false;
    }
    memset( frames, 0, sizeof(*frames) * EffectsEngine::MaxFramesForMixer );
    while ( it != end )
    {
        qint64          start = it.key();
        ClipWorkflow*   cw = it.value();
        //Is the clip supposed to render now?
        if ( start <= currentFrame && currentFrame <= start + cw->getClipHelper()->length() )
        {
            ret = renderClip( cw, currentFrame, start, needRepositioning,
                              renderOneFrame, paused );
            if ( m_trackType == MainWorkflow::VideoTrack )
            {
                frames[frameId] = reinterpret_cast<StackedBuffer<Workflow::Frame*>*>( ret );
                ++frameId;
            }
            else
                m_audioStackedBuffer = reinterpret_cast<StackedBuffer<Workflow::AudioSample*>*>( ret );
        }
        //Is it about to be rendered?
        else if ( start > currentFrame &&
                start - currentFrame < TrackWorkflow::nbFrameBeforePreload )
            preloadClip( cw );
        //Is it supposed to be stopped?
        else
            stopClipWorkflow( cw );
        ++it;
    }
    //Handle mixers:
    if ( m_trackType == MainWorkflow::VideoTrack )
    {
        EffectsEngine::MixerHelper* mixer = EffectsEngine::getMixer( m_mixers, currentFrame );
        if ( mixer != NULL && frames[0] != NULL ) //There's no point using the mixer if there's no frame rendered.
        {
            //FIXME: We don't handle mixer3 yet.
            mixer->effect->process( currentFrame * 1000.0 / m_fps,
                                    frames[0]->get()->buffer(),
                                    frames[1] != NULL ? frames[1]->get()->buffer() : MainWorkflow::blackOutput->buffer(),
                                    NULL, m_mixerBuffer->buffer() );
            m_mixerBuffer->ptsDiff = frames[0]->get()->ptsDiff;
            //The rest of the code uses stackedbuffer, m_mixerBuffer is just a Frame*
            for ( quint32 i = 0; i < EffectsEngine::MaxFramesForMixer; ++i )
            {
                if ( frames[i] != NULL )
                    frames[i]->release();
                else
                    break;
            }
            m_lastFrame = subFrame;
            return m_mixerBuffer;
        }
        else //If no mixer, clean the potentially rendered extra frames.
        {
            for ( quint32 i = 1; i < EffectsEngine::MaxFramesForMixer; ++i )
            {
                if ( frames[i] != NULL )
                    frames[i]->release();
                else
                    break;
            }
            ret = frames[0];
            m_videoStackedBuffer = reinterpret_cast<StackedBuffer<Workflow::Frame*>*>( ret );
        }
    }
    m_lastFrame = subFrame;
    if ( ret == NULL )
        return NULL;
    if ( m_trackType == MainWorkflow::VideoTrack )
        return reinterpret_cast<StackedBuffer<Workflow::Frame*>*>( ret )->get();
    return reinterpret_cast<StackedBuffer<Workflow::AudioSample*>*>( ret )->get();
}

void            TrackWorkflow::moveClip( const QUuid& id, qint64 startingFrame )
{
    QWriteLocker    lock( m_clipsLock );

    QMap<qint64, ClipWorkflow*>::iterator       it = m_clips.begin();
    QMap<qint64, ClipWorkflow*>::iterator       end = m_clips.end();

    while ( it != end )
    {
        if ( it.value()->getClipHelper()->uuid() == id )
        {
            ClipWorkflow* cw = it.value();
            m_clips.erase( it );
            m_clips[startingFrame] = cw;
            cw->requireResync();
            computeLength();
            return ;
        }
        ++it;
    }
}

Clip*       TrackWorkflow::removeClip( const QUuid& id )
{
    QWriteLocker    lock( m_clipsLock );

    QMap<qint64, ClipWorkflow*>::iterator       it = m_clips.begin();
    QMap<qint64, ClipWorkflow*>::iterator       end = m_clips.end();

    while ( it != end )
    {
        if ( it.value()->getClipHelper()->uuid() == id )
        {
            ClipWorkflow*   cw = it.value();
            Clip*           clip = cw->clip();
            m_clips.erase( it );
            stopClipWorkflow( cw );
            computeLength();
            cw->disconnect();
            delete cw;
            return clip;
        }
        ++it;
    }
    return NULL;
}

ClipWorkflow*       TrackWorkflow::removeClipWorkflow( const QUuid& id )
{
    QWriteLocker    lock( m_clipsLock );

    QMap<qint64, ClipWorkflow*>::iterator       it = m_clips.begin();
    QMap<qint64, ClipWorkflow*>::iterator       end = m_clips.end();

    while ( it != end )
    {
        if ( it.value()->getClipHelper()->uuid() == id )
        {
            ClipWorkflow*   cw = it.value();
            cw->disconnect();
            m_clips.erase( it );
            computeLength();
            return cw;

        }
        ++it;
    }
    return NULL;
}

void    TrackWorkflow::save( QXmlStreamWriter& project ) const
{
    QReadLocker     lock( m_clipsLock );

    QMap<qint64, ClipWorkflow*>::const_iterator     it = m_clips.begin();
    QMap<qint64, ClipWorkflow*>::const_iterator     end = m_clips.end();

    for ( ; it != end ; ++it )
    {
        project.writeStartElement( "clip" );
        project.writeAttribute( "startFrame", QString::number( it.key() ) );
        it.value()->save( project );
        project.writeEndElement();
    }
}

void    TrackWorkflow::clear()
{
    QWriteLocker    lock( m_clipsLock );
    QMap<qint64, ClipWorkflow*>::iterator       it = m_clips.begin();
    QMap<qint64, ClipWorkflow*>::iterator       end = m_clips.end();

    for ( ; it != end; ++it )
    {
        ClipWorkflow*   cw = it.value();
        //The clip contained in the trackworkflow will be delete by the undo stack.
        delete cw;
    }
    m_clips.clear();
    m_length = 0;
}

void    TrackWorkflow::adjustClipTime( qint64 currentFrame, qint64 start, ClipWorkflow* cw )
{
    qint64  nbMs = ( currentFrame - start ) / cw->clip()->getMedia()->fps() * 1000;
    qint64  beginInMs = cw->getClipHelper()->begin() / cw->clip()->getMedia()->fps() * 1000;
    qint64  startFrame = beginInMs + nbMs;
    cw->setTime( startFrame, currentFrame );
}

void
TrackWorkflow::renderOneFrame()
{
    QMutexLocker    lock( m_renderOneFrameMutex );
    m_renderOneFrame = true;
}

void
TrackWorkflow::setFullSpeedRender( bool val )
{
    foreach ( ClipWorkflow* cw, m_clips.values() )
    {
        cw->setFullSpeedRender( val );
    }
}

void
TrackWorkflow::muteClip( const QUuid &uuid )
{
    QWriteLocker    lock( m_clipsLock );

    QMap<qint64, ClipWorkflow*>::iterator       it = m_clips.begin();
    QMap<qint64, ClipWorkflow*>::iterator       end = m_clips.end();

    while ( it != end )
    {
        if ( it.value()->getClipHelper()->uuid() == uuid )
        {
            it.value()->mute();
            return ;
        }
        ++it;
    }
    qWarning() << "Failed to mute clip" << uuid << "it probably doesn't exist "
            "in this track";
}

void
TrackWorkflow::unmuteClip( const QUuid &uuid )
{
    QWriteLocker    lock( m_clipsLock );

    QMap<qint64, ClipWorkflow*>::iterator       it = m_clips.begin();
    QMap<qint64, ClipWorkflow*>::iterator       end = m_clips.end();

    while ( it != end )
    {
        if ( it.value()->getClipHelper()->uuid() == uuid )
        {
            it.value()->unmute();
            return ;
        }
        ++it;
    }
    qWarning() << "Failed to unmute clip" << uuid << "it probably doesn't exist "
            "in this track";
}

void
TrackWorkflow::initRender( quint32 width, quint32 height, double fps )
{
    QReadLocker     lock( m_clipsLock );

    m_mixerBuffer->resize( width, height );
    m_fps = fps;
    QMap<qint64, ClipWorkflow*>::iterator       it = m_clips.begin();
    QMap<qint64, ClipWorkflow*>::iterator       end = m_clips.end();
    while ( it != end )
    {
        qint64          start = it.key();
        ClipWorkflow*   cw = it.value();
        if ( start < TrackWorkflow::nbFrameBeforePreload )
            preloadClip( cw );
        ++it;
    }
    EffectsEngine::initMixers( m_mixers, width, height );
}

bool
TrackWorkflow::contains( const QUuid &uuid ) const
{
    QMap<qint64, ClipWorkflow*>::const_iterator       it = m_clips.begin();
    QMap<qint64, ClipWorkflow*>::const_iterator       end = m_clips.end();

    while ( it != end )
    {
        if ( it.value()->getClipHelper()->clip()->uuid() == uuid ||
             it.value()->getClipHelper()->clip()->isChild( uuid ) )
            return true;
        ++it;
    }
    return false;
}

void
TrackWorkflow::stopFrameComputing()
{
    QMap<qint64, ClipWorkflow*>::const_iterator       it = m_clips.begin();
    QMap<qint64, ClipWorkflow*>::const_iterator       end = m_clips.end();

    while ( it != end )
    {
        ClipWorkflow*   cw = it.value();

        cw->getStateLock()->lockForRead();

        if ( cw->getState() == ClipWorkflow::Stopped ||
             cw->getState() == ClipWorkflow::Muted ||
             cw->getState() == ClipWorkflow::Error )
        {
            cw->getStateLock()->unlock();
            return ;
        }
        cw->getStateLock()->unlock();
        cw->stopRenderer();
        ++it;
    }
}
