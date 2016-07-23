/*****************************************************************************
 * MainWorkflow.cpp : Will query all of the track workflows to render the final
 *                    image
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


#include "vlmc.h"
#include "Commands/Commands.h"
#include "Commands/AbstractUndoStack.h"
#include "Backend/MLT/MLTOutput.h"
#include "Backend/MLT/MLTMultiTrack.h"
#include "Backend/MLT/MLTTrack.h"
#include "Renderer/AbstractRenderer.h"
#ifdef WITH_GUI
#include "Gui/WorkflowFileRendererDialog.h"
#endif
#include "Project/Project.h"
#include "Media/Clip.h"
#include "Media/Media.h"
#include "Library/Library.h"
#include "MainWorkflow.h"
#include "Project/Project.h"
#include "TrackWorkflow.h"
#include "Settings/Settings.h"
#include "Tools/VlmcDebug.h"
#include "Tools/RendererEventWatcher.h"
#include "Tools/OutputEventWatcher.h"
#include "Workflow/Types.h"

#include <QMutex>

MainWorkflow::MainWorkflow( Settings* projectSettings, int trackCount ) :
        m_trackCount( trackCount ),
        m_settings( new Settings ),
        m_renderer( new AbstractRenderer ),
        m_multitrack( new Backend::MLT::MLTMultiTrack )
{
    m_renderer->setInput( m_multitrack );

    connect( m_renderer->eventWatcher(), &RendererEventWatcher::lengthChanged, this, &MainWorkflow::lengthChanged );
    connect( m_renderer->eventWatcher(), &RendererEventWatcher::endReached, this, &MainWorkflow::mainWorkflowEndReached );
    connect( m_renderer->eventWatcher(), &RendererEventWatcher::positionChanged, this, [this]( qint64 pos )
    {
        emit frameChanged( pos, Vlmc::Renderer );
    } );

    for ( int i = 0; i < trackCount; ++i )
        m_tracks << new TrackWorkflow( i, m_multitrack );

    m_settings->createVar( SettingValue::List, "tracks", QVariantList(), "", "", SettingValue::Nothing );
    connect( m_settings, &Settings::postLoad, this, &MainWorkflow::postLoad, Qt::DirectConnection );
    connect( m_settings, &Settings::preSave, this, &MainWorkflow::preSave, Qt::DirectConnection );
    projectSettings->addSettings( "Workspace", *m_settings );
}

MainWorkflow::~MainWorkflow()
{
    m_clips.clear();
    for ( auto track : m_tracks )
        delete track;
    delete m_multitrack;
    delete m_renderer;
    delete m_settings;
}

qint64
MainWorkflow::getClipPosition( const QUuid& uuid, unsigned int trackId ) const
{
    Q_ASSERT( trackId < m_trackCount );
    return m_tracks[trackId]->getClipPosition( uuid );
}

void
MainWorkflow::muteTrack( unsigned int trackId, Workflow::TrackType trackType )
{
    Q_ASSERT( trackId < m_trackCount );
    m_tracks[trackId]->mute( true, trackType );
}

void
MainWorkflow::unmuteTrack( unsigned int trackId, Workflow::TrackType trackType )
{
    Q_ASSERT( trackId < m_trackCount );
    m_tracks[trackId]->mute( false, trackType );
}

void
MainWorkflow::muteClip( const QUuid& uuid, unsigned int trackId )
{
    Q_ASSERT( trackId < m_trackCount );
    m_tracks[trackId]->muteClip( uuid );
}

void
MainWorkflow::unmuteClip( const QUuid& uuid, unsigned int trackId )
{
    Q_ASSERT( trackId < m_trackCount );
    m_tracks[trackId]->unmuteClip( uuid );
}

std::shared_ptr<Clip>
MainWorkflow::clip( const QUuid &uuid, unsigned int trackId )
{
    Q_ASSERT( trackId < m_trackCount );
    return m_tracks[trackId]->clip( uuid );
}

void
MainWorkflow::clear()
{
    for ( auto track : m_tracks )
        track->clear();
    emit cleared();
}

void
MainWorkflow::setPosition( qint64 newFrame )
{
    m_renderer->setPosition( newFrame );
}

AbstractRenderer*
MainWorkflow::renderer()
{
    return m_renderer;
}

int
MainWorkflow::getTrackCount() const
{
    return m_trackCount;
}

bool
MainWorkflow::contains( const QUuid &uuid ) const
{
    for ( auto track : m_tracks )
        if ( track->contains( uuid ) == true )
            return true;
    return false;
}

quint32
MainWorkflow::trackCount() const
{
    return m_trackCount;
}

std::shared_ptr<Clip>
MainWorkflow::createClip( const QUuid& uuid, quint32 trackId )
{
    Clip* clip = Core::instance()->library()->clip( uuid );
    if ( clip == nullptr )
    {
        vlmcCritical() << "Couldn't find an acceptable parent to be added.";
        return nullptr;
    }
    auto newClip = std::make_shared<Clip>( clip );
    m_clips.insertMulti( trackId, newClip );
    return newClip;
}

QString
MainWorkflow::addClip( const QString& uuid, quint32 trackId, qint32 pos, bool isAudioClip  )
{
    auto newClip = createClip( uuid, trackId );

    if ( isAudioClip == true )
        newClip->setFormats( Clip::Audio );
    else
        newClip->setFormats( Clip::Video );

    Commands::trigger( new Commands::Clip::Add( newClip, track( trackId ), pos ) );
    emit clipAdded( newClip->uuid().toString() );
    return newClip->uuid().toString();
}

QJsonObject
MainWorkflow::clipInfo( const QString& uuid )
{
    auto lClip = Core::instance()->library()->clip( uuid );
    if ( lClip != nullptr )
    {
        auto h = lClip->toVariant().toHash();
        h["length"] = lClip->length();
        h["name"] = lClip->media()->fileName();
        h["audio"] = lClip->formats().testFlag( Clip::Audio );
        h["video"] = lClip->formats().testFlag( Clip::Video );
        if ( lClip->isRootClip() == true )
        {
            h["begin"] = lClip->begin();
            h["end"] = lClip->end();
        }
        return QJsonObject::fromVariantHash( h );
    }

    for ( auto it = m_clips.begin(); it != m_clips.end(); ++it )
    {
        if ( it.value()->uuid().toString() == uuid )
        {
            auto clip = it.value();
            auto h = clip->toVariant().toHash();
            h["length"] = clip->length();
            h["name"] = clip->media()->fileName();
            h["audio"] = clip->formats().testFlag( Clip::Audio );
            h["video"] = clip->formats().testFlag( Clip::Video );
            h["position"] = track( it.key() )->getClipPosition( uuid );
            h["trackId"] = it.key();
            return QJsonObject::fromVariantHash( h );
        }
    }
    return QJsonObject();
}

void
MainWorkflow::moveClip( quint32 trackId, const QString& uuid, qint64 startFrame )
{
    for ( auto it = m_clips.begin(); it != m_clips.end(); ++it )
    {
        if ( it.value()->uuid().toString() == uuid )
        {
            auto oldTrackId = it.key();
            auto clip = it.value();

            if ( startFrame == getClipPosition( uuid, oldTrackId ) )
                return;

            Commands::trigger( new Commands::Clip::Move( track( oldTrackId ), track( trackId ), clip, startFrame ) );

            m_clips.erase( it );
            m_clips.insertMulti( trackId, clip );
            emit clipMoved( clip->uuid().toString() );
            return;
        }
    }
}

void
MainWorkflow::resizeClip( const QString& uuid, qint64 newBegin, qint64 newEnd, qint64 newPos )
{
    for ( auto it = m_clips.begin(); it != m_clips.end(); ++it )
    {
        if ( it.value()->uuid().toString() == uuid )
        {
            auto trackId = it.key();
            auto clip = it.value();

            Commands::trigger( new Commands::Clip::Resize( track( trackId ), clip, newBegin, newEnd, newPos ) );
            emit clipResized( uuid );
            return;
        }
    }
}

void
MainWorkflow::removeClip( const QString& uuid )
{
    for ( auto it = m_clips.begin(); it != m_clips.end(); ++it )
    {
        if ( it.value()->uuid().toString() == uuid )
        {
            auto trackId = it.key();
            auto clip = it.value();

            Commands::trigger( new Commands::Clip::Remove( clip, track( trackId ) ) );
            emit clipRemoved( uuid );
            return;
        }
    }
}

bool
MainWorkflow::startRenderToFile( const QString &outputFileName, quint32 width, quint32 height,
                                 double fps, const QString &ar, quint32 vbitrate, quint32 abitrate,
                                 quint32 nbChannels, quint32 sampleRate )
{
    m_renderer->stop();

    if ( m_multitrack->playableLength() == 0 )
        return false;

    Backend::MLT::MLTFFmpegOutput output;
    OutputEventWatcher            cEventWatcher;
    output.setCallback( &cEventWatcher );
    output.setTarget( qPrintable( outputFileName ) );
    output.setWidth( width );
    output.setHeight( height );
    output.setFrameRate( fps * 100, 100 );
    auto temp = ar.split( "/" );
    output.setAspectRatio( temp[0].toInt(), temp[1].toInt() );
    output.setVideoBitrate( vbitrate );
    output.setAudioBitrate( abitrate );
    output.setChannels( nbChannels );
    output.setAudioSampleRate( sampleRate );
    output.connect( *m_multitrack );

#ifdef WITH_GUI
    WorkflowFileRendererDialog  dialog( width, height, m_multitrack->playableLength(), m_renderer->eventWatcher() );
    dialog.setModal( true );
    dialog.setOutputFileName( outputFileName );
    connect( &cEventWatcher, &OutputEventWatcher::stopped, &dialog, &WorkflowFileRendererDialog::accept );
    connect( &dialog, &WorkflowFileRendererDialog::stop, this, [&output]{ output.stop(); } );
#endif

    connect( &cEventWatcher, &OutputEventWatcher::stopped, this, [&output]{ output.stop(); } );
    connect( this, &MainWorkflow::mainWorkflowEndReached, this, [&output]{ output.stop(); } );

    m_multitrack->setPosition( 0 );
    output.start();

#ifdef WITH_GUI
    if ( dialog.exec() == QDialog::Rejected )
        return false;
#else
    while ( output.isStopped() == false )
        SleepS( 1 );
#endif
    return true;
}

bool
MainWorkflow::canRender()
{
    return m_multitrack->playableLength() > 0;
}

void
MainWorkflow::preSave()
{
    QVariantList l;
    for ( auto track : m_tracks )
        l << track->toVariant();
    m_settings->value( "tracks" )->set( l );
}

void
MainWorkflow::postLoad()
{
    QVariantList l = m_settings->value( "tracks" )->get().toList();
    for ( int i = 0; i < l.size(); ++i )
        m_tracks[i]->loadFromVariant( l[i] );
}

TrackWorkflow*
MainWorkflow::track( quint32 trackId )
{
    return m_tracks[ trackId ];
}
