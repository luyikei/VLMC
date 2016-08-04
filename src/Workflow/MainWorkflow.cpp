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

#include <QMutex>

MainWorkflow::MainWorkflow( Settings* projectSettings, int trackCount ) :
        m_trackCount( trackCount ),
        m_settings( new Settings ),
        m_renderer( new AbstractRenderer ),
        m_undoStack( new Commands::AbstractUndoStack ),
        m_sequenceWorkflow( new SequenceWorkflow( trackCount ) )
{
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

AbstractRenderer*
MainWorkflow::renderer()
{
    return m_renderer;
}

Backend::IInput*
MainWorkflow::clipInput( const QString& uuid )
{
    auto clip = m_sequenceWorkflow->clip( uuid );
    if ( clip )
        return clip->input();
    return nullptr;
}

Backend::IInput*
MainWorkflow::trackInput( quint32 trackId )
{
    return m_sequenceWorkflow->trackInput( trackId );
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

QString
MainWorkflow::addClip( const QString& uuid, quint32 trackId, qint32 pos, bool isAudioClip  )
{
    auto command = new Commands::Clip::Add( m_sequenceWorkflow, uuid, trackId, pos, isAudioClip );
    trigger( command );
    auto newClip = command->newClip();
    if ( newClip )
        return newClip->uuid().toString();
    return QUuid().toString();
}

QJsonObject
MainWorkflow::clipInfo( const QString& uuid )
{
    auto lClip = Core::instance()->library()->clip( uuid );
    if ( lClip != nullptr )
    {
        auto h = lClip->toVariant().toHash();
        h["length"] = (qint64)( lClip->input()->length() );
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

    auto clip = m_sequenceWorkflow->clip( uuid );
    if ( !clip )
        return QJsonObject();

    auto h = clip->toVariant().toHash();
    h["length"] = (qint64)( clip->input()->length() );
    h["name"] = clip->media()->fileName();
    h["audio"] = clip->formats().testFlag( Clip::Audio );
    h["video"] = clip->formats().testFlag( Clip::Video );
    h["position"] = m_sequenceWorkflow->position( uuid );
    h["trackId"] = m_sequenceWorkflow->trackId( uuid );
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

    auto clipI = clipInput( clipUuid );
    if ( clipI != nullptr )
    {
        trigger( new Commands::Effect::Add( newEffect, clipI ) );
        emit effectsUpdated( clipUuid );
        return newEffect->uuid().toString();
    }

    return QStringLiteral( "" );
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
