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
#include "Workflow/ClipHelper.h"
#include "EffectsEngine/EffectHelper.h"
#include "EffectsEngine/EffectInstance.h"
#include "Workflow/TrackWorkflow.h"

void
Commands::trigger( QUndoCommand* command )
{
    Core::instance()->undoStack()->push( command );
}

Commands::Generic::Generic() :
        m_valid( true )
{
    //This is connected using a direct connection to ensure the view can be refreshed
    //just after the signal has been emited.

    //FIXME: there is no signal retranslateRequired in QUndoStack class
    //       <3 qt4 connects
    connect( Core::instance()->undoStack(), SIGNAL( retranslateRequired() ),
             this, SLOT( retranslate() ), Qt::DirectConnection );
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

Commands::Clip::Add::Add( ClipHelper* ch, TrackWorkflow* tw, qint64 pos ) :
        m_clipHelper( ch ),
        m_trackWorkflow( tw ),
        m_pos( pos )
{
    connect( ch->clip(), &::Clip::destroyed, this, &Add::invalidate );
    retranslate();
}

Commands::Clip::Add::~Add()
{
}

void
Commands::Clip::Add::internalRedo()
{
    m_trackWorkflow->addClip( m_clipHelper, m_pos );
}

void
Commands::Clip::Add::internalUndo()
{
    m_trackWorkflow->removeClip( m_clipHelper->uuid() );
}

void
Commands::Clip::Add::retranslate()
{
    setText( tr( "Adding clip to track %1" ).arg( m_trackWorkflow->trackId() ) );
}

Commands::Clip::Move::Move( TrackWorkflow *oldTrack, TrackWorkflow *newTrack,
                                            ClipHelper *clipHelper, qint64 newPos ) :
    m_oldTrack( oldTrack ),
    m_newTrack( newTrack ),
    m_clipHelper( clipHelper ),
    m_newPos( newPos )

{
    Q_ASSERT( oldTrack->type() == newTrack->type() );

    m_oldPos = oldTrack->getClipPosition( clipHelper->uuid() );
    connect( clipHelper->clip(), SIGNAL( destroyed() ), this, SLOT( invalidate() ) );
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
        ClipWorkflow    *cw = m_oldTrack->removeClipWorkflow( m_clipHelper->uuid() );
        m_newTrack->addClip( cw, m_newPos );
    }
    else
        m_oldTrack->moveClip( m_clipHelper->uuid(), m_newPos );
}

void
Commands::Clip::Move::internalUndo()
{
    if ( m_newTrack != m_oldTrack )
    {
        ClipWorkflow    *cw = m_newTrack->removeClipWorkflow( m_clipHelper->uuid() );
        m_oldTrack->addClip( cw, m_oldPos );
    }
    else
        m_newTrack->moveClip( m_clipHelper->uuid(), m_oldPos );
}

Commands::Clip::Remove::Remove( ClipHelper* ch, TrackWorkflow* tw ) :
        m_clipHelper( ch ), m_trackWorkflow( tw )
{
    connect( ch->clip(), &::Clip::destroyed, this, &Remove::invalidate );
    retranslate();
    m_pos = tw->getClipPosition( ch->uuid() );
}

void
Commands::Clip::Remove::retranslate()
{
   setText( tr( "Removing clip " ) );
}

void
Commands::Clip::Remove::internalRedo()
{
    m_trackWorkflow->removeClip( m_clipHelper->uuid() );
}

void
Commands::Clip::Remove::internalUndo()
{
    m_trackWorkflow->addClip( m_clipHelper, m_pos );
}

Commands::Clip::Resize::Resize( TrackWorkflow* tw, ClipHelper* ch, qint64 newBegin,
                                qint64 newEnd, qint64 newPos ) :
    m_trackWorkflow( tw ),
    m_clipHelper( ch ),
    m_newBegin( newBegin ),
    m_newEnd( newEnd ),
    m_newPos( newPos )
{
    connect( ch->clip(), &::Clip::destroyed, this, &Resize::invalidate );
    m_oldBegin = ch->begin();
    m_oldEnd = ch->end();
    m_oldPos = tw->getClipPosition( ch->uuid() );
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
    if ( m_newBegin != m_newEnd )
    {
        m_trackWorkflow->moveClip( m_clipHelper->uuid(), m_newPos );
    }
    m_clipHelper->setBoundaries( m_newBegin, m_newEnd );
}

void
Commands::Clip::Resize::internalUndo()
{
    if ( m_oldBegin != m_newBegin )
    {
        m_trackWorkflow->moveClip( m_clipHelper->uuid(), m_oldPos );
    }
    m_clipHelper->setBoundaries( m_oldBegin, m_oldEnd );
}

Commands::Clip::Split::Split( TrackWorkflow *tw, ClipHelper *toSplit,
                                             qint64 newClipPos, qint64 newClipBegin ) :
    m_trackWorkflow( tw ),
    m_toSplit( toSplit ),
    m_newClip( nullptr ),
    m_newClipPos( newClipPos ),
    m_newClipBegin( newClipBegin )
{
    connect( toSplit->clip(), &::Clip::destroyed, this, &Split::invalidate );
    m_newClip = new ClipHelper( toSplit->clip(), newClipBegin, toSplit->end() );
    m_oldEnd = toSplit->end();
    retranslate();
}

Commands::Clip::Split::~Split()
{
    delete m_newClip;
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

Commands::Effect::Add::Add( EffectHelper *helper, EffectUser *target ) :
        m_helper( helper ),
        m_target( target )
{
    retranslate();
}

void
Commands::Effect::Add::retranslate()
{
    setText( tr( "Adding effect %1" ).arg( m_helper->effectInstance()->effect()->name() ) );
}

void
Commands::Effect::Add::internalRedo()
{
    m_target->addEffect( m_helper );
}

void
Commands::Effect::Add::internalUndo()
{
    m_target->removeEffect( m_helper );
}

Commands::Effect::Move::Move( EffectHelper *helper, EffectUser *old, EffectUser *newUser,
                              qint64 pos) :
    m_helper( helper ),
    m_old( old ),
    m_new( newUser ),
    m_newPos( pos )
{
    m_oldPos = helper->begin();
    m_oldEnd = helper->end();
    m_newEnd = m_helper->end() - ( m_helper->begin() - pos );
    retranslate();
}

void
Commands::Effect::Move::retranslate()
{
    setText( tr( "Moving effect %1" ).arg( m_helper->effectInstance()->effect()->name() ) );
}

void
Commands::Effect::Move::internalRedo()
{
    if ( m_old != m_new )
    {
        m_old->removeEffect( m_helper );
        m_helper->setBoundaries( m_newPos, m_newEnd );
        m_new->addEffect( m_helper );

    }
    else
        m_new->moveEffect( m_helper, m_newPos );
}

void
Commands::Effect::Move::internalUndo()
{
    if ( m_old != m_new )
    {
        m_new->removeEffect( m_helper );
        m_helper->setBoundaries( m_oldPos, m_oldEnd );
        //This must be called after setting boundaries, as the effect's begin is its begin boundary
        m_old->addEffect( m_helper );
    }
    else
        m_new->moveEffect( m_helper, m_oldPos );
}

Commands::Effect::Resize::Resize( EffectUser *target, EffectHelper *helper, qint64 newBegin, qint64 newEnd ) :
        m_target( target ),
        m_helper( helper ),
        m_newBegin( newBegin ),
        m_newEnd( newEnd )
{
    m_oldBegin = helper->begin();
    m_oldEnd = helper->end();
    retranslate();
}

void
Commands::Effect::Resize::retranslate()
{
    setText( tr( "Resizing effect %1" ).arg( m_helper->effectInstance()->effect()->name() ) );
}

void
Commands::Effect::Resize::internalRedo()
{
    if ( m_newBegin != m_oldBegin )
        m_target->moveEffect( m_helper, m_newBegin );
    m_helper->setBoundaries( m_newBegin, m_newEnd );
}

void
Commands::Effect::Resize::internalUndo()
{
    if ( m_oldBegin != m_newBegin )
        m_target->moveEffect( m_helper, m_oldBegin );
    m_helper->setBoundaries( m_oldBegin, m_oldEnd );
}

Commands::Effect::Remove::Remove( EffectHelper *helper, EffectUser *user ) :
        m_helper( helper ),
        m_user( user )
{
    retranslate();
}

void
Commands::Effect::Remove::retranslate()
{
    setText( tr( "Deleting effect %1" ).arg( m_helper->effectInstance()->effect()->name() ) );
}

void
Commands::Effect::Remove::internalRedo()
{
    m_user->removeEffect( m_helper );
}

void
Commands::Effect::Remove::internalUndo()
{
    m_user->addEffect( m_helper );
}
