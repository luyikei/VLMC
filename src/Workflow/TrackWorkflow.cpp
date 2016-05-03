/*****************************************************************************
 * TrackWorkflow.cpp : Will query the Clip workflow for each successive clip in the track
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


#include "TrackWorkflow.h"

#include "Project/Project.h"
#include "Media/Clip.h"
#include "ClipHelper.h"
#include "AudioClipWorkflow.h"
#include "EffectsEngine/EffectInstance.h"
#include "EffectsEngine/EffectHelper.h"
#include "ImageClipWorkflow.h"
#include "Backend/ISource.h"
#include "Main/Core.h"
#include "Library/Library.h"
#include "MainWorkflow.h"
#include "Media/Media.h"
#include "Types.h"
#include "VideoClipWorkflow.h"
#include "vlmc.h"
#include "Tools/VlmcDebug.h"


#include <QReadLocker>
#include <QMutex>

TrackWorkflow::TrackWorkflow( quint32 trackId  ) :
        m_length( 0 ),
        m_trackType( Workflow::NbTrackType ),
        m_trackId( trackId )
{
    m_clipsLock = new QReadWriteLock;
    m_mixerBuffer = new Workflow::Frame;

    for ( int i = 0; i < Workflow::NbTrackType; ++i )
        m_lastFrame[i] = 0;

    connect( this, SIGNAL( effectAdded( EffectHelper*, qint64 ) ),
             this, SLOT( __effectAdded( EffectHelper*, qint64) ) );
    connect( this, SIGNAL( effectMoved( EffectHelper*, qint64 ) ),
             this, SLOT( __effectMoved( EffectHelper*, qint64 ) ) );
    connect( this, SIGNAL( effectRemoved( QUuid ) ),
             this, SLOT( __effectRemoved(QUuid ) ) );
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
    delete m_mixerBuffer;
    delete m_clipsLock;
}

void
TrackWorkflow::addClip( ClipHelper* ch, qint64 start )
{
    ClipWorkflow* cw;
    if ( ch->clip()->media()->fileType() == Media::FileType::Video )
        // FIXME: This whole if statement will be gone as soon as I implement a united ClipWorkflow,
        //        which can generate both audio and video buffers.
        if ( ch->formats() & ClipHelper::Video )
            cw = new VideoClipWorkflow( ch );
        else if ( ch->formats() & ClipHelper::Audio )
            cw = new AudioClipWorkflow( ch );
        else
            vlmcFatal( "Nothing to render from this clip!" );
    else if ( ch->clip()->media()->fileType() == Media::FileType::Audio )
        cw = new AudioClipWorkflow( ch );
    else if ( ch->clip()->media()->fileType() == Media::FileType::Image )
        cw = new ImageClipWorkflow( ch );
    else
        vlmcFatal( "Unknown file type!" );
    ch->setClipWorkflow( cw );
    addClip( cw, start );
}

void
TrackWorkflow::addClip( ClipWorkflow* cw, qint64 start )
{
    QWriteLocker    lock( m_clipsLock );
    m_clips.insertMulti( start, cw );
    connect( cw, SIGNAL( effectAdded( EffectHelper*, qint64 ) ),
             this, SLOT( __effectAdded( EffectHelper*, qint64 ) ) );
    connect( cw, SIGNAL( effectMoved( EffectHelper*, qint64 ) ),
             this, SLOT( __effectMoved( EffectHelper*, qint64) ) );
    connect( cw, SIGNAL( effectRemoved( QUuid ) ),
             this, SLOT( __effectRemoved( QUuid ) ) );
    // For errors, we don't want this to be called directly from a VLC thread, so we queue it.
    connect( cw, SIGNAL( error( ClipWorkflow* ) ),
             this, SLOT( clipWorkflowFailure( ClipWorkflow* ) ), Qt::QueuedConnection );
    connect( cw->getClipHelper(), SIGNAL( destroyed( QUuid ) ),
             this, SLOT( clipDestroyed( QUuid ) ) );
    emit clipAdded( this, cw->getClipHelper(), start );
    computeLength();
}

//Must be called from a thread safe method (m_clipsLock locked)
void
TrackWorkflow::computeLength()
{
    bool    changed = false;
    if ( m_clips.count() == 0 )
    {
        if ( m_length != 0 )
            changed = true;
        m_length = 0;
    }
    else
    {
        QMap<qint64, ClipWorkflow*>::const_iterator it = m_clips.end() - 1;
        qint64  newLength = it.key() + it.value()->getClipHelper()->length();
        if ( m_length != newLength )
            changed = true;
        m_length = newLength;
    }
    if ( changed == true )
        emit lengthChanged( m_length );
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

ClipHelper*
TrackWorkflow::getClipHelper( const QUuid& uuid )
{
    QMap<qint64, ClipWorkflow*>::const_iterator     it = m_clips.begin();
    QMap<qint64, ClipWorkflow*>::const_iterator     end = m_clips.end();

    while ( it != end )
    {
        if ( it.value()->getClipHelper()->uuid() == uuid )
            return it.value()->getClipHelper();
        ++it;
    }
    return nullptr;
}

Workflow::OutputBuffer*
TrackWorkflow::renderClip( ClipWorkflow* cw, qint64 currentFrame,
                                        qint64 start , bool needRepositioning,
                                        bool renderOneFrame, bool paused )
{
    if ( cw->isMuted() == true )
        return nullptr;

    ClipWorkflow::GetMode       mode = ( paused == false || renderOneFrame == true ?
                                         ClipWorkflow::Pop : ClipWorkflow::Get );

    ClipWorkflow::State state = cw->getState();
    if ( state == ClipWorkflow::Rendering ||
         state == ClipWorkflow::Paused )
    {
        if ( cw->isResyncRequired() == true || needRepositioning == true )
            adjustClipTime( currentFrame, start, cw );
        return cw->getOutput( mode, currentFrame - start );
    }
    else if ( state == ClipWorkflow::Stopped || state == ClipWorkflow::Initializing )
    {
        if ( state == ClipWorkflow::Stopped )
            cw->initialize();
        //If the init failed, don't even try to call getOutput.
        if ( cw->waitForCompleteInit() == false )
            return nullptr;
        //We check for a difference greater than one to avoid false positive when starting.
        if ( (  qAbs(start - currentFrame) > 1 ) || cw->getClipHelper()->begin() != 0 )
        {
            //Clip was not started at its real begining: adjust the position
            adjustClipTime( currentFrame, start, cw );
        }
        return cw->getOutput( mode, currentFrame - start );
    }
    else if ( state == ClipWorkflow::EndReached ||
              state == ClipWorkflow::Error )
    {
        //The stopClipWorkflow() method will take care of EndReached state.
        // When a ClipWorkflow is in error state, we don't want to do anything
    }
    else
    {
        vlmcFatal( "Unexpected state: %d ", state );
    }
    return nullptr;
}

void
TrackWorkflow::preloadClip( ClipWorkflow* cw )
{
    if ( cw->getState() == ClipWorkflow::Stopped )
        cw->initialize();
}

void
TrackWorkflow::stopClipWorkflow( ClipWorkflow* cw )
{
//    vlmcDebug() << "Stopping clip workflow";
    ClipWorkflow::State state = cw->getState();
    if ( state == ClipWorkflow::Stopped ||
         state == ClipWorkflow::Error )
        return ;
    cw->stop();
}

bool
TrackWorkflow::hasNoMoreFrameToRender( qint64 currentFrame ) const
{
    if ( m_clips.size() == 0 )
        return true;
    //This is the last video by chronological order :
    QMap<qint64, ClipWorkflow*>::const_iterator   it = m_clips.end() - 1;
    ClipWorkflow* cw = it.value();
    //Check if the Clip is in error state. If so, don't bother checking anything else.
    if ( cw->getState() == ClipWorkflow::Error )
        return true;
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
    for ( int i = 0; i < Workflow::NbTrackType; ++i )
        m_lastFrame[i] = 0;
    m_isRendering = false;
}

Workflow::OutputBuffer*
TrackWorkflow::getOutput( Workflow::TrackType trackType, qint64 currentFrame, qint64 subFrame, bool paused )
{
    QReadLocker     lock( m_clipsLock );

    QMap<qint64, ClipWorkflow*>::iterator       it = m_clips.begin();
    QMap<qint64, ClipWorkflow*>::iterator       end = m_clips.end();
    bool                                        needRepositioning;
    Workflow::OutputBuffer                      *ret = nullptr;
    Workflow::Frame                             *frames[EffectsEngine::MaxFramesForMixer];
    quint32                                     frameId = 0;
    bool                                        renderOneFrame;

    if ( m_lastFrame[trackType] == -1 )
        m_lastFrame[trackType] = currentFrame;
    renderOneFrame = m_renderOneFrame.testAndSetRelease( true, false );

    {
        // This is a bit hackish : when we want to pop a frame in renderOneFrame mode,
        // we also set the position to avoid the stream to be missynchronized.
        // this frame setting will most likely toggle the next condition as true
        // If this condition is true, the clipworkflow will flush all its buffer
        // as we need to resynchronize after a setTime, so this condition has to remain
        // false. Easy ain't it?
        if ( paused == true && subFrame != m_lastFrame[trackType] && renderOneFrame == false)
            needRepositioning = true;
        else
            needRepositioning = ( qAbs( subFrame - m_lastFrame[trackType] ) > 1 ) ? true : false;
    }
    memset( frames, 0, sizeof(*frames) * EffectsEngine::MaxFramesForMixer );
    while ( it != end )
    {
        qint64          start = it.key();
        ClipWorkflow*   cw = it.value();

        if ( trackType != cw->type() )
        {
            ++it;
            continue ;
        }

        //Is the clip supposed to render now?
        if ( start <= currentFrame && currentFrame <= start + cw->getClipHelper()->length() )
        {
            ret = renderClip( cw, currentFrame, start, needRepositioning,
                              renderOneFrame, paused );
            if ( trackType == Workflow::VideoTrack )
            {
                frames[frameId] = static_cast<Workflow::Frame*>( ret );
                ++frameId;
            }
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
    if ( trackType == Workflow::VideoTrack )
    {
        EffectHelper*   mixer = getMixer( currentFrame );
        if ( mixer != nullptr && frames[0] != nullptr ) //There's no point using the mixer if there's no frame rendered.
        {
            //FIXME: We don't handle mixer3 yet.
            mixer->effectInstance()->process( frames[0]->buffer(),
                                    frames[1] != nullptr ? frames[1]->buffer() : Core::instance()->workflow()->blackOutput()->buffer(),
                                    nullptr, m_mixerBuffer->buffer() );
            m_mixerBuffer->ptsDiff = frames[0]->ptsDiff;
            ret = m_mixerBuffer;
        }
        else //If there's no mixer, just use the first frame, ignore the rest. It will be cleaned by the responsible ClipWorkflow.
            ret = frames[0];
        //Now handle filters :
        quint32     *newFrame = applyFilters( ret != nullptr ? static_cast<const Workflow::Frame*>( ret ) : Core::instance()->workflow()->blackOutput(),
                                                currentFrame );
        if ( newFrame != nullptr )
        {
            if ( ret != nullptr )
                static_cast<Workflow::Frame*>( ret )->setBuffer( newFrame );
            else //Use the m_mixerBuffer as the frame to return. Ugly but avoid another attribute.
            {
                m_mixerBuffer->setBuffer( newFrame );
                ret = m_mixerBuffer;
            }
        }
    }
    m_lastFrame[trackType] = subFrame;
    return ret;
}

void
TrackWorkflow::moveClip( const QUuid& id, qint64 startingFrame )
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
            m_clips.insertMulti( startingFrame, cw );
            cw->requireResync();
            computeLength();
            emit clipMoved( this, cw->getClipHelper()->uuid(), startingFrame );
            return ;
        }
        ++it;
    }
}

void
TrackWorkflow::clipDestroyed( const QUuid& id )
{
    QWriteLocker    lock( m_clipsLock );

    QMap<qint64, ClipWorkflow*>::iterator       it = m_clips.begin();
    QMap<qint64, ClipWorkflow*>::iterator       end = m_clips.end();

    while ( it != end )
    {
        if ( it.value()->getClipHelper()->uuid() == id )
        {
            ClipWorkflow*   cw = it.value();
            m_clips.erase( it );
            stopClipWorkflow( cw );
            computeLength();
            cw->disconnect();
            cw->getClipHelper()->disconnect( this );
            emit clipRemoved( this, id );
            cw->deleteLater();
            return ;
        }
        ++it;
    }
}

void TrackWorkflow::clipWorkflowFailure(ClipWorkflow *cw)
{
    cw->stop();
}

Clip*
TrackWorkflow::removeClip( const QUuid& id )
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
            cw->getClipHelper()->disconnect( this );
            emit clipRemoved( this, cw->getClipHelper()->uuid() );
            cw->deleteLater();
            return clip;
        }
        ++it;
    }
    return nullptr;
}

ClipWorkflow*
TrackWorkflow::removeClipWorkflow( const QUuid& id )
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
            cw->getClipHelper()->disconnect( this );
            emit clipRemoved( this, cw->getClipHelper()->uuid() );
            return cw;
        }
        ++it;
    }
    return nullptr;
}

QVariant
TrackWorkflow::toVariant() const
{
    QVariantList l;
    for ( auto it = m_clips.cbegin(); it != m_clips.cend(); it++ )
    {
        l << QVariantHash{
                    { "clip", (*it)->clip()->uuid() },
                    { "begin", (*it)->getClipHelper()->begin() },
                    { "end", (*it)->getClipHelper()->end() },
                    { "startFrame", it.key() },
                    { "filters", (*it)->toVariant() }
                };
    }
    QVariantHash h{ { "clips", l }, { "filters", EffectUser::toVariant() } };
    return QVariant( h );
}

void
TrackWorkflow::loadFromVariant( const QVariant &variant )
{
    for ( const auto& var : variant.toMap()[ "clips" ].toList() )
    {
        QVariantMap m = var.toMap();
        const QString& uuid     = m["clip"].toString();
        qint64 startFrame       = m["startFrame"].toLongLong();
        qint64 begin            = m["begin"].toLongLong();
        qint64 end              = m["end"].toLongLong();

        if ( uuid.isEmpty() )
        {
            vlmcWarning() << "Invalid clip node";
            return ;
        }

        Clip* c = Core::instance()->library()->clip( uuid );
        if ( c != nullptr )
        {
            ClipHelper  *ch = new ClipHelper( c, begin, end );
            addClip( ch, startFrame );

            ch->clipWorkflow()->loadFromVariant( m["filters"] );
        }
    }
    EffectUser::loadFromVariant( variant.toMap()["filters"] );
}

void
TrackWorkflow::clear()
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

void
TrackWorkflow::adjustClipTime( qint64 currentFrame, qint64 start, ClipWorkflow* cw )
{
    float fps = cw->clip()->media()->source()->fps();
    qint64  nbMs = ( currentFrame - start ) / fps * 1000;
    qint64  beginInMs = cw->getClipHelper()->begin() / fps * 1000;
    qint64  startFrame = beginInMs + nbMs;
    cw->setTime( startFrame );
}

void
TrackWorkflow::renderOneFrame()
{
    m_renderOneFrame.store( true );
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
    vlmcWarning() << "Failed to mute clip" << uuid << "it probably doesn't exist "
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
    vlmcWarning() << "Failed to unmute clip" << uuid << "it probably doesn't exist "
            "in this track";
}

void
TrackWorkflow::initRender( quint32 width, quint32 height )
{
    QReadLocker     lock( m_clipsLock );

    m_mixerBuffer->resize( width, height );
    m_width = width;
    m_height = height;
    m_isRendering = true;
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
    initFilters();
    initMixers();
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

        ClipWorkflow::State state = cw->getState();
        if ( state == ClipWorkflow::Stopped ||
             state == ClipWorkflow::Error )
        {
            return ;
        }
        cw->stop();
        ++it;
    }
}

quint32
TrackWorkflow::trackId() const
{
    return m_trackId;
}

Workflow::TrackType
TrackWorkflow::type() const
{
    return m_trackType;
}

EffectsEngine::EffectList*
TrackWorkflow::filters()
{
    return &m_filters;
}

EffectsEngine::EffectList*
TrackWorkflow::mixers()
{
    return &m_mixers;
}

void
TrackWorkflow::__effectAdded( EffectHelper* helper, qint64 pos )
{
    if ( helper->target()->effectType() == ClipEffectUser )
    {
        ClipWorkflow    *cw = qobject_cast<ClipWorkflow*>( helper->target() );
        Q_ASSERT( cw != nullptr );
        pos += getClipPosition( cw->getClipHelper()->uuid() );
    }
    emit effectAdded( this, helper, pos );
}

void
TrackWorkflow::__effectRemoved( const QUuid& uuid )
{
    emit effectRemoved( this, uuid );
}

void
TrackWorkflow::__effectMoved( EffectHelper* helper, qint64 pos )
{
    if ( helper->target()->effectType() == ClipEffectUser )
    {
        ClipWorkflow    *cw = qobject_cast<ClipWorkflow*>( helper->target() );
        Q_ASSERT( cw != nullptr );
        pos += getClipPosition( cw->getClipHelper()->uuid() );
    }
    emit effectMoved( this, helper->uuid(), pos );
}

qint64
TrackWorkflow::length() const
{
    return m_length;
}

EffectUser::Type
TrackWorkflow::effectType() const
{
    return TrackEffectUser;
}
