/*****************************************************************************
 * AbstractGraphicsMediaItem.h: Base class for media representation
 *****************************************************************************
 * Copyright (C) 2008-2010 VideoLAN
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
#include "TrackWorkflow.h"
#include "GraphicsTrack.h"

#include "Clip.h"
#include "ClipHelper.h"
#include "Commands.h"
#include "Media.h"

#include <QMenu>
#include <QColorDialog>
#include <QGraphicsSceneContextMenuEvent>

#include <QCoreApplication>
#include <QtDebug>

AbstractGraphicsMediaItem::AbstractGraphicsMediaItem( Clip* clip ) :
        m_muted( false )
{
    m_clipHelper = new ClipHelper( clip );
    // Adjust the width
    setWidth( clip->length() );
    // Automatically adjust future changes
    connect( m_clipHelper, SIGNAL( lengthUpdated() ), this, SLOT( adjustLength() ) );
    connect( clip, SIGNAL( unloaded( Clip* ) ),
             this, SLOT( clipDestroyed( Clip* ) ), Qt::DirectConnection );
}

AbstractGraphicsMediaItem::AbstractGraphicsMediaItem( ClipHelper* ch ) :
        m_clipHelper( ch ),
        m_muted( false )
{
    // Adjust the width
    setWidth( ch->length() );
    // Automatically adjust future changes
    connect( ch, SIGNAL( lengthUpdated() ), this, SLOT( adjustLength() ) );
    connect( ch->clip(), SIGNAL( unloaded( Clip* ) ),
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

    QAction* removeAction = menu.addAction( "Remove" );
    QAction* muteAction = menu.addAction( "Mute" );
    muteAction->setCheckable( true );
    muteAction->setChecked( m_muted );

    QAction* linkAction = NULL;
    QAction* unlinkAction = NULL;

    if ( groupItem() )
        unlinkAction = menu.addAction( "Unlink" );
    else
    {
        QList<QGraphicsItem*> items = scene()->selectedItems();
        linkAction = menu.addAction( "Link" );

        if ( items.count() != 2 )
            linkAction->setEnabled( false );
    }

    menu.addSeparator();

    QAction* changeColorAction = menu.addAction( "Set color" );

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
                                                    trackId,
                                                    trackType() );
        }
        else
        {
            tracksView()->m_mainWorkflow->unmuteClip( uuid(),
                                                    trackId,
                                                    trackType() );
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
            track()->trackWorkflow()->moveClip( item1->clipHelper()->uuid(), startPos() );
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
    if ( m_tracksView != NULL )
        m_tracksView->removeClip( clip->uuid() );
}

ClipHelper*
AbstractGraphicsMediaItem::clipHelper()
{
    return m_clipHelper;
}

const ClipHelper*
AbstractGraphicsMediaItem::clipHelper() const
{
    return m_clipHelper;
}

const QUuid&
AbstractGraphicsMediaItem::uuid() const
{
    return m_clipHelper->uuid();
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
    return ( clipHelper()->clip()->getMedia()->fileType() != Media::Image );
}

qint64
AbstractGraphicsMediaItem::maxBegin() const
{
    return ( m_clipHelper->clip()->begin() );
}

qint64
AbstractGraphicsMediaItem::maxEnd() const
{
    return clipHelper()->clip()->end();
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
    return m_clipHelper->begin();
}


qint64
AbstractGraphicsMediaItem::end() const
{
    return m_clipHelper->end();
}

void
AbstractGraphicsMediaItem::triggerMove( EffectUser *target, qint64 startPos )
{
    TrackWorkflow   *tw = qobject_cast<TrackWorkflow*>( target );
    if ( tw == NULL )
        return ;
    Commands::trigger( new Commands::Clip::Move( m_oldTrack, tw, m_clipHelper, startPos ) );
}

Workflow::Helper*
AbstractGraphicsMediaItem::helper()
{
    return m_clipHelper;
}

void
AbstractGraphicsMediaItem::triggerResize( EffectUser *target, Workflow::Helper *helper,
                                           qint64 newBegin, qint64 newEnd, qint64 pos )
{
    ClipHelper  *clipHelper = qobject_cast<ClipHelper*>( helper );
    if ( clipHelper == NULL )
        return ;
    TrackWorkflow   *tw = qobject_cast<TrackWorkflow*>( target );
    if ( tw == NULL )
        return ;
    Commands::trigger( new Commands::Clip::Resize( tw, clipHelper, newBegin,
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
