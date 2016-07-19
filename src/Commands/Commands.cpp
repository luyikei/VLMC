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

#include "config.h"

#include "Commands.h"
#include "Project/Project.h"
#include "Main/Core.h"
#include "Media/Clip.h"
#include "EffectsEngine/EffectHelper.h"
#include "Workflow/TrackWorkflow.h"
#include "AbstractUndoStack.h"
#include "Backend/IFilter.h"

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

Commands::Clip::Add::Add( std::shared_ptr<::Clip> const& clip, TrackWorkflow* tw, qint64 pos ) :
        m_clip( clip ),
        m_trackWorkflow( tw ),
        m_pos( pos )
{
    connect( clip.get(), &::Clip::destroyed, this, &Add::invalidate );
    retranslate();
}

void
Commands::Clip::Add::internalRedo()
{
    m_trackWorkflow->addClip( m_clip, m_pos );
}

void
Commands::Clip::Add::internalUndo()
{
    m_trackWorkflow->removeClip( m_clip->uuid() );
}

void
Commands::Clip::Add::retranslate()
{
    setText( tr( "Adding clip to track %1" ).arg( m_trackWorkflow->trackId() ) );
}

Commands::Clip::Move::Move( TrackWorkflow *oldTrack, TrackWorkflow *newTrack,
                            std::shared_ptr<::Clip> const& clip, qint64 newPos ) :
    m_oldTrack( oldTrack ),
    m_newTrack( newTrack ),
    m_clip( clip ),
    m_newPos( newPos )

{
    m_oldPos = oldTrack->getClipPosition( m_clip->uuid() );
    connect( m_clip.get(), SIGNAL( destroyed() ), this, SLOT( invalidate() ) );
    retranslate();
}

void
Commands::Clip::Move::retranslate()
{
    if ( m_oldTrack != m_newTrack )
        setText( tr( "Moving clip from track %1 to %2" ).arg(
                QString::number( m_oldTrack->trackId() ), QString::number( m_newTrack->trackId() ) ) );
    else
        setText( QObject::tr( "Moving clip" ) );
}

void
Commands::Clip::Move::internalRedo()
{
    if ( m_newTrack != m_oldTrack )
    {
        m_clip = m_oldTrack->removeClip( m_clip->uuid() );
        m_newTrack->addClip( m_clip, m_newPos );
    }
    else
        m_oldTrack->moveClip( m_clip->uuid(), m_newPos );
}

void
Commands::Clip::Move::internalUndo()
{
    if ( m_newTrack != m_oldTrack )
    {
        m_clip = m_newTrack->removeClip( m_clip->uuid() );
        m_oldTrack->addClip( m_clip, m_oldPos );
    }
    else
        m_newTrack->moveClip( m_clip->uuid(), m_oldPos );
}

Commands::Clip::Remove::Remove( std::shared_ptr<::Clip> const& clip, TrackWorkflow* tw ) :
        m_clip( clip ), m_trackWorkflow( tw )
{
    connect( clip.get(), &::Clip::destroyed, this, &Remove::invalidate );
    retranslate();
    m_pos = tw->getClipPosition( clip->uuid() );
}

void
Commands::Clip::Remove::retranslate()
{
   setText( tr( "Removing clip " ) );
}

void
Commands::Clip::Remove::internalRedo()
{
    m_trackWorkflow->removeClip( m_clip->uuid() );
}

void
Commands::Clip::Remove::internalUndo()
{
    m_trackWorkflow->addClip( m_clip, m_pos );
}

Commands::Clip::Resize::Resize( TrackWorkflow* tw, std::shared_ptr<::Clip> const& clip, qint64 newBegin,
                                qint64 newEnd, qint64 newPos ) :
    m_trackWorkflow( tw ),
    m_clip( clip ),
    m_newBegin( newBegin ),
    m_newEnd( newEnd ),
    m_newPos( newPos )
{
    connect( clip.get(), &::Clip::destroyed, this, &Resize::invalidate );
    m_oldBegin = clip->begin();
    m_oldEnd = clip->end();
    m_oldPos = tw->getClipPosition( clip->uuid() );
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
    m_trackWorkflow->resizeClip( m_clip->uuid(), m_newBegin, m_newEnd, m_newPos );
}

void
Commands::Clip::Resize::internalUndo()
{
    m_trackWorkflow->resizeClip( m_clip->uuid(), m_oldBegin, m_oldEnd, m_oldPos );
}

Commands::Clip::Split::Split( TrackWorkflow *tw, std::shared_ptr<::Clip> const& toSplit,
                                             qint64 newClipPos, qint64 newClipBegin ) :
    m_trackWorkflow( tw ),
    m_toSplit( toSplit ),
    m_newClip( nullptr ),
    m_newClipPos( newClipPos ),
    m_newClipBegin( newClipBegin )
{
    connect( toSplit.get(), &::Clip::destroyed, this, &Split::invalidate );
    m_newClip = std::make_shared<::Clip>( toSplit.get(), newClipBegin, toSplit->end() );
    m_oldEnd = toSplit->end();
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
    //If we don't remove 1, the clip will end exactly at the starting frame (ie. they will
    //be rendering at the same time)
    m_toSplit->setEnd( m_newClipBegin );
    m_trackWorkflow->addClip( m_newClip, m_newClipPos );
}

void
Commands::Clip::Split::internalUndo()
{
    m_trackWorkflow->removeClip( m_newClip->uuid() );
    m_toSplit->setEnd( m_oldEnd );
}

Commands::Clip::Link::Link( std::shared_ptr<::Clip> const& clipA, std::shared_ptr<::Clip> const& clipB )
    : m_clipA( clipA )
    , m_clipB( clipB )
{
    connect( m_clipA.get(), &::Clip::destroyed, this, &Link::invalidate );
    connect( m_clipB.get(), &::Clip::destroyed, this, &Link::invalidate );
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
    m_clipA->setLinkedClipUuid( m_clipB->uuid() );
    m_clipB->setLinkedClipUuid( m_clipA->uuid() );
    m_clipA->setLinked( true );
    m_clipB->setLinked( true );
}

void
Commands::Clip::Link::internalUndo()
{
    m_clipA->setLinked( false );
    m_clipB->setLinked( false );
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
