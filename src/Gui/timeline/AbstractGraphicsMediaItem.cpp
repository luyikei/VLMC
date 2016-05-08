/*****************************************************************************
 * AbstractGraphicsMediaItem.h: Base class for media representation
 *****************************************************************************
 * Copyright (C) 2008-2016 VideoLAN
 *
 * Authors: Ludovic Fauvet <etix@l0cal.com>
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

#include "AbstractGraphicsMediaItem.h"
#include "TracksView.h"
#include "TracksScene.h"
#include "Workflow/TrackWorkflow.h"
#include "GraphicsTrack.h"

#include "Media/Clip.h"
#include "Commands/Commands.h"
#include "Media/Media.h"

#include <QMenu>
#include <QColorDialog>
#include <QGraphicsSceneContextMenuEvent>

#include <QCoreApplication>

AbstractGraphicsMediaItem::AbstractGraphicsMediaItem( Clip* clip ) :
        m_muted( false )
{
    m_clip = new Clip( clip );
    // Adjust the width
    setWidth( clip->length() );
    // Automatically adjust future changes
    connect( m_clip, SIGNAL( lengthUpdated() ), this, SLOT( adjustLength() ) );
    connect( clip, SIGNAL( unloaded( Clip* ) ),
             this, SLOT( clipDestroyed( Clip* ) ), Qt::DirectConnection );
}

AbstractGraphicsMediaItem::~AbstractGraphicsMediaItem()
{
    ungroup();
}

void
AbstractGraphicsMediaItem::contextMenuEvent( QGraphicsSceneContextMenuEvent* event )
{
    if ( !tracksView() )
        return;

    QMenu menu( tracksView() );

    QAction* removeAction = menu.addAction( tr( "Remove" ) );
    QAction* muteAction = menu.addAction( tr( "Mute" ) );
    muteAction->setCheckable( true );
    muteAction->setChecked( m_muted );

    QAction* linkAction = nullptr;
    QAction* unlinkAction = nullptr;

    if ( groupItem() )
        unlinkAction = menu.addAction( tr( "Unlink" ) );
    else
    {
        QList<QGraphicsItem*> items = scene()->selectedItems();
        linkAction = menu.addAction( tr( "Link" ) );

        if ( items.count() != 2 )
            linkAction->setEnabled( false );
    }

    menu.addSeparator();

    QAction* changeColorAction = menu.addAction( tr( "Set color" ) );

    QAction* selectedAction = menu.exec( event->screenPos() );

    if ( !selectedAction )
        return;

    if ( selectedAction == removeAction )
        scene()->askRemoveSelectedItems();
    else if ( selectedAction == muteAction )
    {
        qint32      trackId = trackNumber();
        Q_ASSERT( trackId >= 0 );
        if ( ( m_muted = muteAction->isChecked() ) )
        {
            tracksView()->m_mainWorkflow->muteClip( uuid(),
                                                    trackId );
        }
        else
        {
            tracksView()->m_mainWorkflow->unmuteClip( uuid(),
                                                    trackId );
        }
    }
    else if ( selectedAction == linkAction )
    {
        QList<QGraphicsItem*> items = scene()->selectedItems();

        AbstractGraphicsMediaItem* item1;
        AbstractGraphicsMediaItem* item2;

        item1 = dynamic_cast<AbstractGraphicsMediaItem*>( items.at( 0 ) );
        item2 = dynamic_cast<AbstractGraphicsMediaItem*>( items.at( 1 ) );

        Q_ASSERT( item1 );
        Q_ASSERT( item2 );

        if ( item1 == this )
            item1 = item2;
        //From here, the item we click on is "this" and the item to group is "item1"

        if ( item1->trackType() != trackType() )
        {
            qint32      item1TrackId = item1->trackNumber();
            Q_ASSERT( item1TrackId >= 0 );
            item1->group( this );
            tracksView()->moveItem( item1, item1TrackId , startPos() );
            track()->trackWorkflow()->moveClip( item1->clip()->uuid(), startPos() );
        }
    }
    else if ( selectedAction == unlinkAction )
    {
        QList<QGraphicsItem*> items = scene()->selectedItems();

        AbstractGraphicsMediaItem* item;
        item = dynamic_cast<AbstractGraphicsMediaItem*>( items.at( 0 ) );

        item->ungroup();
    }
    else if ( selectedAction == changeColorAction )
    {
        m_itemColor = QColorDialog::getColor( m_itemColor, tracksView() );
        update();
    }

}

void
AbstractGraphicsMediaItem::clipDestroyed( Clip* clip )
{
    if ( m_tracksView != nullptr )
        m_tracksView->removeClip( clip->uuid() );
}

Clip*
AbstractGraphicsMediaItem::clip()
{
    return m_clip;
}

const Clip*
AbstractGraphicsMediaItem::clip() const
{
    return m_clip;
}

const QUuid&
AbstractGraphicsMediaItem::uuid() const
{
    return m_clip->uuid();
}

void
AbstractGraphicsMediaItem::setEmphasized( bool value )
{
    if ( value == true )
        setScale( 1.2 );
    else
        setScale( 1.0 );
}

bool
AbstractGraphicsMediaItem::hasResizeBoundaries() const
{
    return ( clip()->media()->fileType() != Media::Image );
}

qint64
AbstractGraphicsMediaItem::maxBegin() const
{
    // Assume that the clip always has a parent.
    return clip()->parent()->begin();
}

qint64
AbstractGraphicsMediaItem::maxEnd() const
{
    // Assume that the clip always has a parent.
    return clip()->parent()->end();
}

void
AbstractGraphicsMediaItem::hoverEnterEvent( QGraphicsSceneHoverEvent* event )
{
    TracksView* tv = tracksView();
    if ( tv )
    {
        switch ( tv->tool() )
        {
            case TOOL_DEFAULT:
            setCursor( Qt::OpenHandCursor );
            break;

            case TOOL_CUT:
            setCursor( QCursor( QPixmap( ":/images/editcut" ) ) );
            break;
        }
    }
    QGraphicsItem::hoverEnterEvent( event );
}

void
AbstractGraphicsMediaItem::mousePressEvent( QGraphicsSceneMouseEvent* event )
{
    if ( !tracksView() ) return;

    if ( tracksView()->tool() == TOOL_DEFAULT )
    {
        if ( resizeZone( event->pos() ) )
            setCursor( Qt::SizeHorCursor );
        else
            setCursor( Qt::ClosedHandCursor );
    }
    else if ( tracksView()->tool() == TOOL_CUT )
        emit split( this, qRound64( event->pos().x() ) );
}

qint64
AbstractGraphicsMediaItem::begin() const
{
    return m_clip->begin();
}


qint64
AbstractGraphicsMediaItem::end() const
{
    return m_clip->end();
}

void
AbstractGraphicsMediaItem::triggerMove( EffectUser *target, qint64 startPos )
{
    TrackWorkflow   *tw = qobject_cast<TrackWorkflow*>( target );
    if ( tw == nullptr )
        return ;
    Commands::trigger( new Commands::Clip::Move( m_oldTrack, tw, m_clip, startPos ) );
}

Workflow::Helper*
AbstractGraphicsMediaItem::helper()
{
    return m_clip;
}

void
AbstractGraphicsMediaItem::triggerResize( EffectUser *target, Workflow::Helper *helper,
                                           qint64 newBegin, qint64 newEnd, qint64 pos )
{
    Clip  *clip = qobject_cast<Clip*>( helper );
    if ( clip == nullptr )
        return ;
    TrackWorkflow   *tw = qobject_cast<TrackWorkflow*>( target );
    if ( tw == nullptr )
        return ;
    Commands::trigger( new Commands::Clip::Resize( tw, clip, newBegin,
                                                               newEnd, pos ) );
}

qint64
AbstractGraphicsMediaItem::itemHeight() const
{
    return 35;
}

qint32
AbstractGraphicsMediaItem::zSelected() const
{
    return 100;
}

qint32
AbstractGraphicsMediaItem::zNotSelected() const
{
    return 50;
}

void
AbstractGraphicsMediaItem::setStartPos( qint64 position )
{
    emit moved( position );
    AbstractGraphicsItem::setStartPos( position );
}
