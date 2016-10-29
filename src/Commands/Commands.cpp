/*****************************************************************************
 * Commands.cpp: Contains all the implementation of VLMC commands.
 *****************************************************************************
 * Copyright (C) 2008-2016 VideoLAN
 *
 * Authors: Hugo Beauzée-Luyssen <hugo@beauzee.fr>
 *          Ludovic Fauvet <etix@l0cal.com>
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

#include "Commands.h"
#include "Project/Project.h"
#include "Main/Core.h"
#include "Media/Clip.h"
#include "Media/Media.h"
#include "EffectsEngine/EffectHelper.h"
#include "Workflow/SequenceWorkflow.h"
#include "Workflow/MainWorkflow.h"
#include "AbstractUndoStack.h"
#include "Backend/IFilter.h"
#include "Library/Library.h"

Commands::Generic::Generic() :
        m_valid( true )
{
    //This is connected using a direct connection to ensure the view can be refreshed
    //just after the signal has been emited.

    //FIXME: there is no signal retranslateRequired in QUndoStack class
    //       <3 qt4 connects
    // connect( Core::instance()->undoStack(), SIGNAL( retranslateRequired() ),
    //          this, SLOT( retranslate() ), Qt::DirectConnection );
}

void
Commands::Generic::invalidate()
{
    setText( tr( "Invalid action" ) );
    m_valid = false;
    emit invalidated();
}

bool
Commands::Generic::isValid() const
{
    return m_valid;
}

#ifndef HAVE_GUI
void
Commands::Generic::setText( const QString& text )
{
    m_text = text;
}

QString
Commands::Generic::text() const
{
    return m_text;
}
#endif

void
Commands::Generic::redo()
{
    if ( m_valid == true )
        internalRedo();
}

void
Commands::Generic::undo()
{
    if ( m_valid == true )
        internalUndo();
}

Commands::Clip::Add::Add( std::shared_ptr<SequenceWorkflow> const& workflow,
                          const QUuid& uuid, quint32 trackId, qint32 pos ) :
        m_workflow( workflow ),
        m_libraryUuid( uuid ),
        m_trackId( trackId ),
        m_pos( pos )
{
    retranslate();
}

void
Commands::Clip::Add::internalRedo()
{
    auto clip = Core::instance()->library()->clip( m_libraryUuid );
    if ( clip == nullptr )
    {
        invalidate();
        return;
    }
    if ( clip->media()->hasAudioTracks() )
    {
        // In case we are redoing, we are feeding the addClip method with the previously generated UUID
        // so that future operations could still rely on the same instance being present
        m_audioInstanceUuid = m_workflow->addClip( clip, m_trackId, m_pos, m_audioInstanceUuid, true );
        if ( m_audioInstanceUuid.isNull() == true )
            invalidate();
    }
    if ( clip->media()->hasVideoTracks() )
    {
        m_videoInstanceUuid = m_workflow->addClip( clip, m_trackId, m_pos, m_videoInstanceUuid, false );
        if ( m_videoInstanceUuid.isNull() == true )
            invalidate();
    }
    if ( m_audioInstanceUuid.isNull() == false && m_videoInstanceUuid.isNull() == false )
        m_workflow->linkClips( m_audioInstanceUuid, m_videoInstanceUuid );
}

void
Commands::Clip::Add::internalUndo()
{
    if ( m_audioInstanceUuid.isNull() == false &&
         m_workflow->removeClip( m_audioInstanceUuid ) == nullptr )
            invalidate();
    if ( m_videoInstanceUuid.isNull() == false &&
         m_workflow->removeClip( m_videoInstanceUuid ) == nullptr )
            invalidate();
}

void
Commands::Clip::Add::retranslate()
{
    setText( tr( "Adding clip to track %1" ).arg( m_trackId ) );
}

Commands::Clip::Move::Move(  std::shared_ptr<SequenceWorkflow> const& workflow,
                             const QString& uuid, quint32 trackId, qint64 pos ) :
    m_workflow( workflow ),
    m_infos( {{ uuid, trackId, workflow->trackId( uuid ),pos, workflow->position( uuid ) }} )
{
    retranslate();
}

void
Commands::Clip::Move::retranslate()
{
    setText( QObject::tr( "Moving clip(s)", "", m_infos.count() ) );
}

int
Commands::Clip::Move::id() const
{
    return static_cast<int>( Commands::Id::Move );
}

bool
Commands::Clip::Move::mergeWith( const QUndoCommand* command )
{
    auto cmd = static_cast<const Move*>( command );
    if ( cmd->m_infos.count() > 1 )
        return false;
    const auto& clip = m_workflow->clip( m_infos[0].uuid );
    const auto& linkedClips = clip->linkedClips;
    if ( linkedClips.contains( cmd->m_infos[0].uuid ) == false )
        return false;
    m_infos += cmd->m_infos[0];
    return true;
}

void
Commands::Clip::Move::internalRedo()
{
    for ( const auto& info : m_infos )
    {
        if ( m_workflow->moveClip( info.uuid, info.newTrackId, info.newPos ) == false )
            invalidate();
    }
}

void
Commands::Clip::Move::internalUndo()
{
    for ( const auto& info : m_infos )
    {
        if ( m_workflow->moveClip( info.uuid, info.oldTrackId, info.oldPos ) == false )
            invalidate();
    }
}

Commands::Clip::Remove::Remove( std::shared_ptr<SequenceWorkflow> const& workflow,
                                const QUuid& uuid ) :
        m_workflow( workflow ),
        m_clip( workflow->clip( uuid ) ),
        m_trackId( workflow->trackId( uuid ) ),
        m_pos( workflow->position( uuid ) )
{
    retranslate();
}

void
Commands::Clip::Remove::retranslate()
{
   setText( tr( "Removing clip " ) );
}

void
Commands::Clip::Remove::internalRedo()
{
    if ( m_clip == nullptr )
    {
        invalidate();
        return;
    }
    m_clip = m_workflow->removeClip( m_clip->uuid );
    if ( m_clip == nullptr )
        invalidate();
}

void
Commands::Clip::Remove::internalUndo()
{
    if ( m_clip == nullptr )
    {
        invalidate();
        return;
    }
    auto ret = m_workflow->addClip( m_clip->clip, m_trackId, m_pos, m_clip->uuid, m_clip->isAudio );
    if ( ret.isNull() == true )
        invalidate();
}

Commands::Clip::Resize::Resize( std::shared_ptr<SequenceWorkflow> const& workflow,
                                const QUuid& uuid, qint64 newBegin, qint64 newEnd, qint64 newPos ) :
    m_workflow( workflow ),
    m_clip( workflow->clip( uuid ) ),
    m_newBegin( newBegin ),
    m_newEnd( newEnd ),
    m_newPos( newPos )
{
    if ( m_clip->uuid.isNull() == true )
    {
        invalidate();
        return;
    }
    m_oldBegin = m_clip->clip->begin();
    m_oldEnd = m_clip->clip->end();
    m_oldPos = workflow->trackId( uuid );
    retranslate();

}

void
Commands::Clip::Resize::retranslate()
{
    setText( tr( "Resizing clip" ) );
}

void
Commands::Clip::Resize::internalRedo()
{
    bool ret = m_workflow->resizeClip( m_clip->uuid, m_newBegin, m_newEnd, m_newPos );
    if ( ret == false )
        invalidate();
}

void
Commands::Clip::Resize::internalUndo()
{
    bool ret = m_workflow->resizeClip( m_clip->uuid, m_oldBegin, m_oldEnd, m_oldPos );
    if ( ret == false )
        invalidate();
}

Commands::Clip::Split::Split( std::shared_ptr<SequenceWorkflow> const& workflow,
                              const QUuid& uuid, qint64 newClipPos, qint64 newClipBegin ) :
    m_workflow( workflow ),
    m_toSplit( workflow->clip( uuid ) ),
    m_trackId( workflow->trackId( uuid ) ),
    m_newClip( nullptr ),
    m_newClipPos( newClipPos ),
    m_newClipBegin( newClipBegin )
{
    if ( !m_toSplit )
    {
        invalidate();
        retranslate();
        return;
    }
    m_newClip = m_toSplit->clip->media()->cut( newClipBegin - m_toSplit->clip->begin(),
                                         m_toSplit->clip->end() - m_toSplit->clip->begin() );
    m_oldEnd = m_toSplit->clip->end();
    retranslate();
}

void
Commands::Clip::Split::retranslate()
{
    setText( tr("Splitting clip") );
}

void
Commands::Clip::Split::internalRedo()
{
    if ( !m_toSplit )
    {
        invalidate();
        return;
    }
    //If we don't remove 1, the clip will end exactly at the starting frame (ie. they will
    //be rendering at the same time)
    bool ret = m_workflow->resizeClip( m_toSplit->uuid, m_toSplit->clip->begin(),
                                       m_newClipBegin - 1, m_workflow->position( m_toSplit->uuid ) );
    if ( ret == false )
    {
        invalidate();
        return;
    }

    m_newClipUuid = m_workflow->addClip( m_newClip, m_trackId, m_newClipPos, m_newClipUuid, m_toSplit->isAudio );
    if ( m_newClipUuid.isNull() == true )
        invalidate();
    emit Core::instance()->workflow()->clipResized( m_toSplit->uuid.toString() );
}

void
Commands::Clip::Split::internalUndo()
{
    if ( m_workflow->removeClip( m_newClip->uuid() ) == nullptr )
    {
        invalidate();
        return;
    }
    else
        emit Core::instance()->workflow()->clipRemoved( m_newClip->uuid().toString() );
    m_workflow->resizeClip( m_toSplit->clip->uuid(), m_toSplit->clip->begin(),
                            m_oldEnd, m_workflow->position( m_toSplit->uuid ) );
    emit Core::instance()->workflow()->clipResized( m_toSplit->uuid.toString() );
}

Commands::Clip::Link::Link( std::shared_ptr<SequenceWorkflow> const& workflow,
                            const QUuid& clipA, const QUuid& clipB )
    : m_workflow( workflow )
    , m_clipA( clipA )
    , m_clipB( clipB )
{
    retranslate();
}

void
Commands::Clip::Link::retranslate()
{
    setText( tr( "Linking clip" ) );
}

void
Commands::Clip::Link::internalRedo()
{
    auto ret = m_workflow->linkClips( m_clipA, m_clipB );
    if ( ret == false )
        invalidate();
}

void
Commands::Clip::Link::internalUndo()
{
    auto ret = m_workflow->unlinkClips( m_clipA, m_clipB );
    if ( ret == false )
        invalidate();
}

Commands::Clip::Unlink::Unlink( std::shared_ptr<SequenceWorkflow> const& workflow,
                            const QUuid& clipA, const QUuid& clipB )
    : m_workflow( workflow )
    , m_clipA( clipA )
    , m_clipB( clipB )
{
    retranslate();
}

void
Commands::Clip::Unlink::retranslate()
{
    setText( tr( "Unlinking clips" ) );
}

void
Commands::Clip::Unlink::internalRedo()
{
    auto ret = m_workflow->unlinkClips( m_clipA, m_clipB );
    if ( ret == false )
        invalidate();
}

void
Commands::Clip::Unlink::internalUndo()
{
    auto ret = m_workflow->linkClips( m_clipA, m_clipB );
    if ( ret == false )
        invalidate();
}

Commands::Effect::Add::Add( std::shared_ptr<EffectHelper> const& helper, Backend::IInput* target )
    : m_helper( helper )
    , m_target( target )
{
    retranslate();
}

void
Commands::Effect::Add::retranslate()
{
    setText( tr( "Adding effect %1" ).arg( QString::fromStdString( m_helper->filterInfo()->name() ) ) );
}

void
Commands::Effect::Add::internalRedo()
{
    m_target->attach( *m_helper->filter() );
}

void
Commands::Effect::Add::internalUndo()
{
    m_target->detach( *m_helper->filter() );
}

Commands::Effect::Move::Move( std::shared_ptr<EffectHelper> const& helper, std::shared_ptr<Backend::IInput> const& from, Backend::IInput* to,
                              qint64 pos)
    : m_helper( helper )
    , m_from( from )
    , m_to( to )
    , m_newPos( pos )
{
    m_oldPos = helper->begin();
    m_oldEnd = helper->end();
    m_newEnd = helper->end() - helper->begin() + pos;
    retranslate();
}

void
Commands::Effect::Move::retranslate()
{
    setText( tr( "Moving effect %1" ).arg( QString::fromStdString( m_helper->filterInfo()->name() ) ) );
}

void
Commands::Effect::Move::internalRedo()
{
    if ( m_from->sameClip( *m_to ) == false )
    {
        m_from->detach( *m_helper->filter() );
        m_to->attach( *m_helper->filter() );
        m_helper->setBoundaries( m_newPos, m_newEnd );

    }
    else
        m_helper->setBoundaries( m_newPos, m_newEnd );
}

void
Commands::Effect::Move::internalUndo()
{
    if ( m_from->sameClip( *m_to ) == false )
    {
        m_to->detach( *m_helper->filter() );
        m_from->attach( *m_helper->filter() );
        m_helper->setBoundaries( m_oldPos, m_oldEnd );
    }
    else
        m_helper->setBoundaries( m_oldPos, m_oldEnd );
}

Commands::Effect::Resize::Resize( std::shared_ptr<EffectHelper> const& helper, qint64 newBegin, qint64 newEnd )
    : m_helper( helper )
    , m_newBegin( newBegin )
    , m_newEnd( newEnd )
{
    m_oldBegin = helper->begin();
    m_oldEnd = helper->end();
    retranslate();
}

void
Commands::Effect::Resize::retranslate()
{
    setText( tr( "Resizing effect %1" ).arg( QString::fromStdString( m_helper->filterInfo()->name() ) ) );
}

void
Commands::Effect::Resize::internalRedo()
{
    m_helper->setBoundaries( m_newBegin, m_newEnd );
}

void
Commands::Effect::Resize::internalUndo()
{
    m_helper->setBoundaries( m_oldBegin, m_oldEnd );
}

Commands::Effect::Remove::Remove( std::shared_ptr<EffectHelper> const& helper )
    : m_helper( helper )
    , m_target( helper->filter()->input() )
{

}

void
Commands::Effect::Remove::retranslate()
{
    setText( tr( "Deleting effect %1" ).arg( QString::fromStdString( m_helper->filterInfo()->name() ) ) );
}

void
Commands::Effect::Remove::internalRedo()
{
    m_target->detach( *m_helper->filter() );
}

void
Commands::Effect::Remove::internalUndo()
{
    m_helper->setTarget( m_target.get() );
}
