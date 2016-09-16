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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "vlmc.h"
#include "Commands/Commands.h"
#include "Commands/AbstractUndoStack.h"
#include "Backend/MLT/MLTOutput.h"
#include "Backend/MLT/MLTMultiTrack.h"
#include "Backend/MLT/MLTTrack.h"
#include "Renderer/AbstractRenderer.h"
#ifdef HAVE_GUI
#include "EffectsEngine/EffectHelper.h"
#include "Gui/effectsengine/EffectStack.h"
#include "Gui/WorkflowFileRendererDialog.h"
#endif
#include "Project/Project.h"
#include "Media/Clip.h"
#include "Media/Media.h"
#include "Library/Library.h"
#include "MainWorkflow.h"
#include "Project/Project.h"
#include "SequenceWorkflow.h"
#include "Settings/Settings.h"
#include "Tools/VlmcDebug.h"
#include "Tools/RendererEventWatcher.h"
#include "Tools/OutputEventWatcher.h"
#include "Workflow/Types.h"
#include "ThumbnailWorker.h"

#include <QMutex>

MainWorkflow::MainWorkflow( Settings* projectSettings, int trackCount ) :
        m_trackCount( trackCount ),
        m_settings( new Settings ),
        m_renderer( new AbstractRenderer ),
        m_undoStack( new Commands::AbstractUndoStack ),
        m_sequenceWorkflow( new SequenceWorkflow( trackCount ) )
{
    connect( m_sequenceWorkflow.get(), &SequenceWorkflow::clipAdded, this, &MainWorkflow::clipAdded );
    connect( m_sequenceWorkflow.get(), &SequenceWorkflow::clipRemoved, this, &MainWorkflow::clipRemoved );
    connect( m_sequenceWorkflow.get(), &SequenceWorkflow::clipLinked, this, &MainWorkflow::clipLinked );
    connect( m_sequenceWorkflow.get(), &SequenceWorkflow::clipUnlinked, this, &MainWorkflow::clipUnlinked );
    connect( m_sequenceWorkflow.get(), &SequenceWorkflow::clipMoved, this, &MainWorkflow::clipMoved );
    connect( m_sequenceWorkflow.get(), &SequenceWorkflow::clipResized, this, &MainWorkflow::clipResized );
    m_renderer->setInput( m_sequenceWorkflow->input() );

    connect( m_renderer->eventWatcher(), &RendererEventWatcher::lengthChanged, this, &MainWorkflow::lengthChanged );
    connect( m_renderer->eventWatcher(), &RendererEventWatcher::endReached, this, &MainWorkflow::mainWorkflowEndReached );
    connect( m_renderer->eventWatcher(), &RendererEventWatcher::positionChanged, this, [this]( qint64 pos )
    {
        emit frameChanged( pos, Vlmc::Renderer );
    } );

    m_settings->createVar( SettingValue::List, "tracks", QVariantList(), "", "", SettingValue::Nothing );
    connect( m_settings, &Settings::postLoad, this, &MainWorkflow::postLoad, Qt::DirectConnection );
    connect( m_settings, &Settings::preSave, this, &MainWorkflow::preSave, Qt::DirectConnection );
    projectSettings->addSettings( "Workspace", *m_settings );

    connect( m_undoStack.get(), &Commands::AbstractUndoStack::cleanChanged, this, &MainWorkflow::cleanChanged );
}

MainWorkflow::~MainWorkflow()
{
    m_renderer->stop();
    delete m_renderer;
    delete m_settings;
}

void
MainWorkflow::unmuteTrack( unsigned int trackId, Workflow::TrackType trackType )
{
    // TODO
}

void
MainWorkflow::muteClip( const QUuid& uuid, unsigned int trackId )
{
    // TODO
}

void
MainWorkflow::unmuteClip( const QUuid& uuid, unsigned int trackId )
{
    // TODO
}

std::shared_ptr<Clip>
MainWorkflow::clip( const QUuid &uuid, unsigned int trackId )
{
    // TODO
}

void
MainWorkflow::trigger( Commands::Generic* command )
{
    m_undoStack->push( command );
}

void
MainWorkflow::clear()
{
    m_sequenceWorkflow->clear();
    emit cleared();
}

void
MainWorkflow::setClean()
{
    m_undoStack->setClean();
}

void
MainWorkflow::setPosition( qint64 newFrame )
{
    m_renderer->setPosition( newFrame );
}

void
MainWorkflow::setFps( double fps )
{
    Backend::instance()->profile().setFrameRate( fps * 100, 100 );
    emit fpsChanged( fps );
}

void
MainWorkflow::showEffectStack()
{
#ifdef HAVE_GUI
    auto w = new EffectStack( m_sequenceWorkflow->input() );
    w->show();
#endif
}

void
MainWorkflow::showEffectStack( quint32 trackId )
{
#ifdef HAVE_GUI
    auto w = new EffectStack( m_sequenceWorkflow->trackInput( trackId ) );
    w->show();
#endif
}

void
MainWorkflow::showEffectStack( const QString& uuid )
{
#ifdef HAVE_GUI
    auto w = new EffectStack( m_sequenceWorkflow->clip( uuid )->clip->input() );
    connect( w, &EffectStack::finished, Core::instance()->workflow(), [uuid]{ emit Core::instance()->workflow()->effectsUpdated( uuid ); } );
    w->show();
#endif
}

AbstractRenderer*
MainWorkflow::renderer()
{
    return m_renderer;
}

Commands::AbstractUndoStack*
MainWorkflow::undoStack()
{
    return m_undoStack.get();
}

int
MainWorkflow::getTrackCount() const
{
    return m_trackCount;
}

bool
MainWorkflow::contains( const QUuid &uuid ) const
{
    auto clip = m_sequenceWorkflow->clip( uuid );
    return !clip == false;
}

quint32
MainWorkflow::trackCount() const
{
    return m_trackCount;
}

void
MainWorkflow::addClip( const QString& uuid, quint32 trackId, qint32 pos )
{
    vlmcDebug() << "Adding clip:" << uuid;
    auto command = new Commands::Clip::Add( m_sequenceWorkflow, uuid, trackId, pos );
    trigger( command );
}

QJsonObject
MainWorkflow::clipInfo( const QString& uuid )
{
    auto c = m_sequenceWorkflow->clip( uuid );
    if ( c != nullptr )
    {
        auto clip = c->clip;
        auto h = clip->toVariant().toHash();
        h["uuid"] = uuid;
        h["length"] = (qint64)( clip->input()->length() );
        h["name"] = clip->media()->title();
        h["audio"] = c->isAudio;
        h["position"] = m_sequenceWorkflow->position( uuid );
        h["trackId"] = m_sequenceWorkflow->trackId( uuid );
        h["filters"] = EffectHelper::toVariant( clip->input() );
        return QJsonObject::fromVariantHash( h );
    }
    return QJsonObject();
}

QJsonObject
MainWorkflow::libraryClipInfo( const QString& uuid )
{
    auto c = Core::instance()->library()->clip( uuid );
    if ( c == nullptr )
        return {};
    auto h = c->toVariant().toHash();
    h["length"] = (qint64)( c->input()->length() );
    h["name"] = c->media()->title();
    h["audio"] = c->media()->hasAudioTracks();
    h["video"] = c->media()->hasVideoTracks();
    h["begin"] = c->begin();
    h["end"] = c->end();
    h["uuid"] = "libraryUuid";
    return QJsonObject::fromVariantHash( h );
}

void
MainWorkflow::moveClip( const QString& uuid, quint32 trackId, qint64 startFrame )
{
    trigger( new Commands::Clip::Move( m_sequenceWorkflow, uuid, trackId, startFrame ) );
}

void
MainWorkflow::resizeClip( const QString& uuid, qint64 newBegin, qint64 newEnd, qint64 newPos )
{
    trigger( new Commands::Clip::Resize( m_sequenceWorkflow, uuid, newBegin, newEnd, newPos ) );
}

void
MainWorkflow::removeClip( const QString& uuid )
{
    trigger( new Commands::Clip::Remove( m_sequenceWorkflow, uuid ) );
}

void
MainWorkflow::splitClip( const QUuid& uuid, qint64 newClipPos, qint64 newClipBegin )
{
    trigger( new Commands::Clip::Split( m_sequenceWorkflow, uuid, newClipPos, newClipBegin ) );
}

void
MainWorkflow::linkClips( const QString& uuidA, const QString& uuidB )
{
    trigger( new Commands::Clip::Link( m_sequenceWorkflow, uuidA, uuidB ) );
}

QString
MainWorkflow::addEffect( const QString &clipUuid, const QString &effectId )
{
    std::shared_ptr<EffectHelper> newEffect;

    try
    {
        newEffect.reset( new EffectHelper( effectId ) );
    }
    catch( Backend::InvalidServiceException& e )
    {
        return QStringLiteral( "" );
    }

    auto clip = m_sequenceWorkflow->clip( clipUuid );
    if ( clip && clip->clip->input() )
    {
        trigger( new Commands::Effect::Add( newEffect, clip->clip->input() ) );
        emit effectsUpdated( clipUuid );
        return newEffect->uuid().toString();
    }

    return QStringLiteral( "" );
}

void
MainWorkflow::takeThumbnail( const QString& uuid, quint32 pos )
{
    vlmcDebug() << "Generating thumbnail for" << uuid;
    // We need to fetch the clip from the library. This clip is being added to the library
    // and doesn't have an instance ID yet
    auto swClip = m_sequenceWorkflow->clip( uuid );
    auto clip = swClip->clip;
    auto worker = new ThumbnailWorker( uuid, clip->media()->mrl(),
                                       pos, clip->input()->width(), clip->input()->height() );
    auto t = new QThread;
    worker->moveToThread( t );
    connect( t, &QThread::started, worker, &ThumbnailWorker::run );
    connect( worker, &ThumbnailWorker::imageReady, this, &MainWorkflow::thumbnailUpdated, Qt::DirectConnection );
    connect( worker, &ThumbnailWorker::imageReady, t, &QThread::quit );
    connect( t, &QThread::finished, worker, &ThumbnailWorker::deleteLater );
    connect( t, &QThread::finished, t, &QThread::deleteLater );
    t->start();
}

bool
MainWorkflow::startRenderToFile( const QString &outputFileName, quint32 width, quint32 height,
                                 double fps, const QString &ar, quint32 vbitrate, quint32 abitrate,
                                 quint32 nbChannels, quint32 sampleRate )
{
    m_renderer->stop();

    if ( canRender() == false )
        return false;

    Backend::MLT::MLTFFmpegOutput output;
    auto input = m_sequenceWorkflow->input();
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
    output.connect( *input );

#ifdef HAVE_GUI
    WorkflowFileRendererDialog  dialog( width, height, input->playableLength(), m_renderer->eventWatcher() );
    dialog.setModal( true );
    dialog.setOutputFileName( outputFileName );
    connect( &cEventWatcher, &OutputEventWatcher::stopped, &dialog, &WorkflowFileRendererDialog::accept );
    connect( &dialog, &WorkflowFileRendererDialog::stop, this, [&output]{ output.stop(); } );
    connect( m_renderer->eventWatcher(), &RendererEventWatcher::positionChanged, &dialog,
             [this, input, &dialog, width, height]( qint64 pos )
    {
        // Update the preview per five seconds
        if ( pos % qRound( input->fps() * 5 ) == 0 )
        {
            dialog.updatePreview( input->image( width, height ) );
        }
    });
#endif

    connect( &cEventWatcher, &OutputEventWatcher::stopped, this, [&output]{ output.stop(); } );
    connect( this, &MainWorkflow::mainWorkflowEndReached, this, [&output]{ output.stop(); } );

    input->setPosition( 0 );
    output.start();

#ifdef HAVE_GUI
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
    return m_sequenceWorkflow->input()->playableLength() > 0;
}

void
MainWorkflow::preSave()
{
    m_settings->value( "tracks" )->set( m_sequenceWorkflow->toVariant() );
}

void
MainWorkflow::postLoad()
{
    m_sequenceWorkflow->loadFromVariant( m_settings->value( "tracks" )->get() );
}
