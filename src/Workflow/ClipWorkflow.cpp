/*****************************************************************************
 * ClipWorkflow.cpp : Clip workflow. Will extract frames from a media
 *****************************************************************************
 * Copyright (C) 2008-2014 VideoLAN
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

#include <QMutex>
#include <QReadWriteLock>
#include <QStringBuilder>
#include <QWaitCondition>

#include "vlmc.h"
#include "Media/Clip.h"
#include "ClipHelper.h"
#include "ClipWorkflow.h"
#include "Backend/ISource.h"
#include "Backend/ISourceRenderer.h"
#include "Media/Media.h"
#include "Tools/RendererEventWatcher.h"
#include "Workflow/Types.h"

#include "Tools/VlmcDebug.h"

ClipWorkflow::ClipWorkflow( ClipHelper* ch )
    : m_renderer( NULL )
    , m_eventWatcher( NULL )
    , m_clipHelper( ch )
    , m_state( ClipWorkflow::Stopped )
{
    m_stateLock = new QReadWriteLock;
    m_initWaitCond = new QWaitCondition;
    m_renderLock = new QMutex;
    m_renderWaitCond = new QWaitCondition;
    m_eventWatcher = new RendererEventWatcher;
}

ClipWorkflow::~ClipWorkflow()
{
    stop();
    delete m_eventWatcher;
    delete m_renderWaitCond;
    delete m_renderLock;
    delete m_initWaitCond;
    delete m_stateLock;
}

void
ClipWorkflow::initialize()
{
    QWriteLocker lock( m_stateLock );
    m_state = ClipWorkflow::Initializing;

    delete m_renderer;
    m_renderer = m_clipHelper->clip()->getMedia()->source()->createRenderer( m_eventWatcher );

    preallocate();
    initializeInternals();

    m_currentPts = -1;
    m_previousPts = -1;
    m_pauseDuration = -1;

    //Use QueuedConnection to avoid getting called from intf-event callback, as
    //we will trigger intf-event callback as well when setting time for this clip,
    //thus resulting in a deadlock.
    connect( m_eventWatcher, SIGNAL( playing() ), this, SLOT( loadingComplete() ), Qt::QueuedConnection );
    connect( m_eventWatcher, SIGNAL( endReached() ), this, SLOT( clipEndReached() ), Qt::DirectConnection );
    connect( m_eventWatcher, SIGNAL( errorEncountered() ), this, SLOT( errorEncountered() ) );
    connect( m_eventWatcher, &RendererEventWatcher::stopped, this, &ClipWorkflow::mediaPlayerStopped );
    m_renderer->start();
}

void
ClipWorkflow::loadingComplete()
{
    adjustBegin();
    disconnect( m_eventWatcher, SIGNAL( playing() ), this, SLOT( loadingComplete() ) );
    connect( m_eventWatcher, SIGNAL( playing() ), this, SLOT( mediaPlayerUnpaused() ), Qt::DirectConnection );
    connect( m_eventWatcher, SIGNAL( paused() ), this, SLOT( mediaPlayerPaused() ), Qt::DirectConnection );
    QWriteLocker lock( m_stateLock );
    m_isRendering = true;
    m_state = Rendering;
    m_initWaitCond->wakeAll();
}

void
ClipWorkflow::adjustBegin()
{
    if ( m_clipHelper->clip()->getMedia()->fileType() == Media::Video ||
         m_clipHelper->clip()->getMedia()->fileType() == Media::Audio )
    {
        m_renderer->setTime( m_clipHelper->begin() /
                                m_clipHelper->clip()->getMedia()->source()->fps() * 1000 );
    }
}

ClipWorkflow::State
ClipWorkflow::getState() const
{
    return m_state;
}

void
ClipWorkflow::clipEndReached()
{
    m_state = EndReached;
}

void
ClipWorkflow::stop()
{
    QWriteLocker    lockState( m_stateLock );

    //Let's make sure the ClipWorkflow isn't beeing stopped from another thread.
    if ( m_renderer && m_state != Stopped )
    {
        m_renderer->stop();
    }
}

void
ClipWorkflow::setTime( qint64 time )
{
    vlmcDebug() << "Setting ClipWorkflow" << m_clipHelper->uuid() << "time:" << time;
    m_renderer->setTime( time );
    resyncClipWorkflow();
    m_renderer->setPause( false );
}

bool
ClipWorkflow::waitForCompleteInit()
{
    QReadLocker lock( m_stateLock );

    if ( m_state != ClipWorkflow::Rendering && m_state != ClipWorkflow::Error )
    {
        m_initWaitCond->wait( m_stateLock );

        if ( m_state != ClipWorkflow::Rendering )
            return false;
    }
    return true;
}

void
ClipWorkflow::postGetOutput()
{
    //If we're running out of computed buffers, refill our stack.
    if ( getNbComputedBuffers() < getMaxComputedBuffers() / 3 )
        m_renderer->setPause( false );
}

void
ClipWorkflow::commonUnlock()
{
    //Don't test using availableBuffer, as it may evolve if a buffer is required while
    //no one is available : we would spawn a new buffer, thus modifying the number of available buffers
    if ( getNbComputedBuffers() >= getMaxComputedBuffers() )
    {
        m_renderer->setPause( true );
    }
}

void
ClipWorkflow::computePtsDiff( qint64 pts )
{
    if ( m_pauseDuration != -1 )
    {
        //No need to check for m_currentPtr before, as we can't start in paused mode.
        //so m_currentPts will not be -1
        m_previousPts = m_currentPts + m_pauseDuration;
        m_pauseDuration = -1;
    }
    else
        m_previousPts = m_currentPts;
    m_currentPts = qMax( pts, m_previousPts );
}

void
ClipWorkflow::mediaPlayerPaused()
{
    m_state = ClipWorkflow::Paused;
    m_beginPausePts = mdate();
}

void
ClipWorkflow::mediaPlayerUnpaused()
{
    m_state = ClipWorkflow::Rendering;
    m_pauseDuration = mdate() - m_beginPausePts;
}

void
ClipWorkflow::mediaPlayerStopped()
{
    m_eventWatcher->disconnect();
    if ( m_state != Error )
        m_state = Stopped;
    flushComputedBuffers();
    m_isRendering = false;

    m_initWaitCond->wakeAll();
    m_renderWaitCond->wakeAll();
}

void
ClipWorkflow::resyncClipWorkflow()
{
    flushComputedBuffers();
    m_previousPts = -1;
    m_currentPts = -1;
}

void
ClipWorkflow::setFullSpeedRender( bool val )
{
    m_fullSpeedRender = val;
}

void
ClipWorkflow::mute()
{
    stop();
    m_muted = true;
}

void
ClipWorkflow::unmute()
{
    m_muted = false;
}

bool
ClipWorkflow::isMuted() const
{
    return m_muted;
}

void
ClipWorkflow::requireResync()
{
    m_resyncRequired = true;
}

bool
ClipWorkflow::isResyncRequired()
{
    if ( m_resyncRequired == true )
    {
        m_resyncRequired = false;
        return true;
    }
    return false;
}

void
ClipWorkflow::errorEncountered()
{
    m_state = Error;
    emit error( this );
}

bool
ClipWorkflow::shouldRender() const
{
    ClipWorkflow::State state = m_state;
    return ( state != ClipWorkflow::Error &&
             state != ClipWorkflow::Stopped);
}

void
ClipWorkflow::save( QXmlStreamWriter &project ) const
{
    project.writeAttribute( "uuid", m_clipHelper->clip()->fullId() );
    project.writeAttribute( "begin", QString::number( m_clipHelper->begin() ) );
    project.writeAttribute( "end", QString::number( m_clipHelper->end() ) );
    project.writeAttribute( "helper", m_clipHelper->uuid().toString() );
    saveFilters( project );
}

qint64
ClipWorkflow::length() const
{
    return m_clipHelper->length();
}

EffectUser::Type
ClipWorkflow::effectType() const
{
    return EffectUser::ClipEffectUser;
}
