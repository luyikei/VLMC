/*****************************************************************************
 * Commands.cpp: Contains all the implementation of VLMC commands.
 *****************************************************************************
 * Copyright (C) 2008-2010 VideoLAN
 *
 * Authors: Hugo Beauzee-Luyssen <hugo@vlmc.org>
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
#include "Clip.h"
#include "ClipHelper.h"
#include "Commands.h"
#include "MainWorkflow.h"
#include "TrackWorkflow.h"

#ifdef WITH_GUI
# include "UndoStack.h"

void Commands::trigger( QUndoCommand* command )
{
    UndoStack::getInstance()->push( command );
}
#else
void Commands::trigger( Commands::Generic* command )
{
    command->redo();
}
#endif



Commands::MainWorkflow::AddClip::AddClip( ClipHelper* ch, TrackWorkflow* tw, qint64 pos ) :
        m_clipHelper( ch ),
        m_trackWorkflow( tw ),
        m_pos( pos )
{
    setText( QObject::tr( "Adding clip to track %1" ).arg( tw->trackId() ) );
}

Commands::MainWorkflow::AddClip::~AddClip()
{
}

void Commands::MainWorkflow::AddClip::redo()
{
    m_trackWorkflow->addClip( m_clipHelper, m_pos );
}

void Commands::MainWorkflow::AddClip::undo()
{
    m_trackWorkflow->removeClip( m_clipHelper->uuid() );
}

Commands::MainWorkflow::MoveClip::MoveClip( TrackWorkflow *oldTrack, TrackWorkflow *newTrack,
                                            ClipHelper *clipHelper, qint64 newPos ) :
    m_oldTrack( oldTrack ),
    m_newTrack( newTrack ),
    m_clipHelper( clipHelper ),
    m_newPos( newPos )

{
    Q_ASSERT( oldTrack->type() == newTrack->type() );

    if ( oldTrack != newTrack )
        setText( QObject::tr( "Moving clip from track %1 to %2" ).arg(
                QString::number( oldTrack->trackId() ), QString::number( newTrack->trackId() ) ) );
    else
        setText( QObject::tr( "Moving clip" ) );
    m_oldPos = oldTrack->getClipPosition( clipHelper->uuid() );
}

void Commands::MainWorkflow::MoveClip::redo()
{
    if ( m_newTrack != m_oldTrack )
    {
        m_oldTrack->removeClip( m_clipHelper->uuid() );
        m_newTrack->addClip( m_clipHelper, m_newPos );
    }
    else
        m_oldTrack->moveClip( m_clipHelper->uuid(), m_newPos );
}

void Commands::MainWorkflow::MoveClip::undo()
{
    if ( m_newTrack != m_oldTrack )
    {
        m_oldTrack->removeClip( m_clipHelper->uuid() );
        m_newTrack->addClip( m_clipHelper, m_oldPos );
    }
    else
        m_newTrack->moveClip( m_clipHelper->uuid(), m_oldPos );
}

Commands::MainWorkflow::RemoveClip::RemoveClip( ClipHelper* ch, TrackWorkflow* tw ) :
        m_clipHelper( ch ), m_trackWorkflow( tw )
{
    setText( QObject::tr( "Remove clip" ) );
    m_pos = tw->getClipPosition( ch->uuid() );
}

void Commands::MainWorkflow::RemoveClip::redo()
{
    m_trackWorkflow->removeClip( m_clipHelper->uuid() );
}
void Commands::MainWorkflow::RemoveClip::undo()
{
    m_trackWorkflow->addClip( m_clipHelper, m_pos );
}

Commands::MainWorkflow::ResizeClip::ResizeClip( TrackWorkflow* tw,
                                                ClipHelper* ch,
                                                qint64 newBegin, qint64 newEnd,
                                                qint64 newPos ) :
    m_trackWorkflow( tw ),
    m_clipHelper( ch ),
    m_newBegin( newBegin ),
    m_newEnd( newEnd ),
    m_newPos( newPos )
{
    m_oldBegin = ch->begin();
    m_oldEnd = ch->end();
    m_oldPos = tw->getClipPosition( ch->uuid() );
    setText( QObject::tr( "Resizing clip" ) );
}

void Commands::MainWorkflow::ResizeClip::redo()
{
    if ( m_newBegin != m_newEnd )
    {
        m_trackWorkflow->moveClip( m_clipHelper->uuid(), m_newPos );
    }
    m_clipHelper->setBoundaries( m_newBegin, m_newEnd );
}

void Commands::MainWorkflow::ResizeClip::undo()
{
    if ( m_oldBegin != m_newBegin )
    {
        m_trackWorkflow->moveClip( m_clipHelper->uuid(), m_oldPos );
    }
    m_clipHelper->setBoundaries( m_oldBegin, m_oldEnd );
}

Commands::MainWorkflow::SplitClip::SplitClip( ClipHelper* toSplit, quint32 trackId,
                           qint64 newClipPos, qint64 newClipBegin, Workflow::TrackType trackType ) :
    m_toSplit( toSplit ),
    m_newClip( NULL ),
    m_trackId( trackId ),
    m_newClipPos( newClipPos ),
    m_newClipBegin( newClipBegin ),
    m_trackType( trackType )
{
    setText( QObject::tr("Splitting clip") );
}

Commands::MainWorkflow::SplitClip::~SplitClip()
{
    if ( m_newClip != NULL )
        delete m_newClip;
}

void    Commands::MainWorkflow::SplitClip::redo()
{
//    m_newClip = ::MainWorkflow::getInstance()->split( m_toSplit, m_newClip, m_trackId, m_newClipPos, m_newClipBegin, m_trackType );
}

void    Commands::MainWorkflow::SplitClip::undo()
{
//    ::MainWorkflow::getInstance()->unsplit( m_toSplit, m_newClip, m_trackId, m_trackType );
}
