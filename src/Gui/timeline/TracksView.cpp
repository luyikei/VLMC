/*****************************************************************************
 * TracksView.cpp: QGraphicsView that contains the TracksScene
 *****************************************************************************
 * Copyright (C) 2008-2014 VideoLAN
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

#include "TracksView.h"

#include "ClipHelper.h"
#include "ClipWorkflow.h"
#include "Commands.h"
#include "EffectHelper.h"
#include "GraphicsMovieItem.h"
#include "GraphicsAudioItem.h"
#include "GraphicsEffectItem.h"
#include "GraphicsCursorItem.h"
#include "GraphicsTrack.h"
#include "Helper.h"
#include "ISource.h"
#include "Library.h"
#include "Media.h"
//Ugly part {
#include "Timeline.h"
#include "TracksRuler.h"
//} this should be fixed, it breaks the design
#include "TrackWorkflow.h"
#include "UndoStack.h"
#include "VlmcDebug.h"
#include "WorkflowRenderer.h"

#include <QGraphicsLinearLayout>
#include <QGraphicsWidget>
#include <QGraphicsRectItem>
#include <QMouseEvent>
#include <QScrollBar>
#include <QWheelEvent>
#include <QMimeData>

TracksView::TracksView( QGraphicsScene *scene, MainWorkflow *mainWorkflow,
                        WorkflowRenderer *renderer, QWidget *parent )
    : QGraphicsView( scene, parent ),
    m_scene( scene ),
    m_mainWorkflow( mainWorkflow ),
    m_renderer( renderer )
{
    //TODO should be defined by the settings
    m_tracksHeight = 35;

    m_numAudioTrack = 0;
    m_numVideoTrack = 0;
    m_dragVideoItem = NULL;
    m_dragAudioItem = NULL;
    m_dragEffectItem = NULL;
    m_lastKnownTrack = NULL;
    m_effectTarget = NULL;
    m_action = None;
    m_actionRelativeX = -1;
    m_actionItem = NULL;
    m_tool = TOOL_DEFAULT;

    setMouseTracking( true );
    setAcceptDrops( true );
    setContentsMargins( 0, 0, 0, 0 );
    setFrameStyle( QFrame::NoFrame );
    setAlignment( Qt::AlignLeft | Qt::AlignTop );
    setCacheMode( QGraphicsView::CacheBackground );

    m_cursorLine = new GraphicsCursorItem( QPen( QColor( 220, 30, 30 ) ) );

    m_scene->addItem( m_cursorLine );

    connect( m_cursorLine, SIGNAL( cursorMoved(qint64) ),
             this, SLOT( ensureCursorVisible() ) );

    for ( quint32 type = Workflow::VideoTrack; type < Workflow::NbTrackType; ++type )
    {
        for ( quint32 i = 0; i < m_mainWorkflow->trackCount(); ++i )
        {
            TrackWorkflow   *tw = m_mainWorkflow->track( static_cast<Workflow::TrackType>( type ), i );
            //Clips part:
            connect( tw, SIGNAL( clipAdded( TrackWorkflow*, Workflow::Helper*, qint64 ) ),
                     this, SLOT( addItem( TrackWorkflow*, Workflow::Helper*, qint64 ) ) );
            connect( tw, SIGNAL( clipRemoved( TrackWorkflow*, const QUuid& ) ),
                     this, SLOT( removeItem( TrackWorkflow*, const QUuid& ) ) );
            connect( tw, SIGNAL( clipMoved( TrackWorkflow*, const QUuid&, qint64 ) ),
                     this, SLOT( moveItem( TrackWorkflow*, const QUuid&, qint64 ) ) );
            //Effect part:
            connect( tw, SIGNAL( effectAdded( TrackWorkflow*, Workflow::Helper*, qint64 ) ),
                     this, SLOT(addItem( TrackWorkflow*, Workflow::Helper*, qint64 ) ), Qt::QueuedConnection );
            connect( tw, SIGNAL( effectRemoved( TrackWorkflow*, QUuid ) ),
                     this, SLOT( removeItem( TrackWorkflow*, QUuid ) ), Qt::QueuedConnection );
            connect( tw, SIGNAL( effectMoved( TrackWorkflow*, QUuid, qint64 ) ),
                     this, SLOT( moveItem( TrackWorkflow*, QUuid, qint64 ) ), Qt::QueuedConnection );

        }
    }
}

void
TracksView::createLayout()
{
    m_layout = new QGraphicsLinearLayout( Qt::Vertical );
    m_layout->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );
    m_layout->setContentsMargins( 0, 0, 0, 0 );
    m_layout->setSpacing( 0 );
    m_layout->setPreferredWidth( 0 );

    QGraphicsWidget *container = new QGraphicsWidget();
    container->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );
    container->setContentsMargins( 0, 0, 0, 0 );
    container->setLayout( m_layout );

    // Create the initial layout
    // - 1 video track
    // - a separator
    // - 1 audio track
    addTrack( Workflow::VideoTrack );

    m_separator = new QGraphicsWidget();
    m_separator->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
    m_separator->setPreferredHeight( 20 );
    m_layout->insertItem( 1, m_separator );

    addTrack( Workflow::AudioTrack );

    m_scene->addItem( container );

    setSceneRect( m_layout->contentsRect() );
}

void
TracksView::addTrack( Workflow::TrackType type )
{
    GraphicsTrack *track = new GraphicsTrack( type,
                                              type == Workflow::VideoTrack ? m_numVideoTrack : m_numAudioTrack );
    track->setHeight( tracksHeight() );
    m_layout->insertItem( type == Workflow::VideoTrack ? 0 : 1000, track );
    m_layout->activate();
    m_cursorLine->setHeight( m_layout->contentsRect().height() );
    m_scene->invalidate(); // Redraw the background

    if ( type == Workflow::VideoTrack )
    {
        m_numVideoTrack++;
        emit videoTrackAdded( track );
    }
    else
    {
        m_numAudioTrack++;
        emit audioTrackAdded( track );
    }

}

void
TracksView::removeVideoTrack()
{
    Q_ASSERT( m_numVideoTrack > 0 );

    QGraphicsLayoutItem *item = m_layout->itemAt( 0 );
    m_layout->removeItem( item );
    m_layout->activate();
    m_scene->invalidate(); // Redraw the background
    m_cursorLine->setHeight( m_layout->contentsRect().height() );
    m_numVideoTrack--;
    emit videoTrackRemoved();
    delete item;
}

void
TracksView::removeAudioTrack()
{
    Q_ASSERT( m_numAudioTrack > 0 );

    QGraphicsLayoutItem *item = m_layout->itemAt( m_layout->count() - 1 );
    m_layout->removeItem( item );
    m_layout->activate();
    m_scene->invalidate(); // Redraw the background
    m_cursorLine->setHeight( m_layout->contentsRect().height() );
    m_numAudioTrack--;
    emit audioTrackRemoved();
    delete item;
}

void
TracksView::clear()
{
    m_layout->removeItem( m_separator );

    while ( m_layout->count() > 0 )
        delete m_layout->itemAt( 0 );

    m_layout->addItem( m_separator );

    m_numAudioTrack = 0;
    m_numVideoTrack = 0;

    addTrack( Workflow::VideoTrack );
    addTrack( Workflow::AudioTrack );
    m_itemsLoaded.clear();

    updateDuration();
}

void
TracksView::removeClip( const QUuid& uuid  )
{
    // Get the list of all items in the timeline
    QList<AbstractGraphicsItem*> items = timelineItems();

    // Iterate over each item to check if their parent's uuid
    // is the one we would like to remove.
    foreach( AbstractGraphicsItem *item, items )
    {
        if ( item->uuid() == uuid )
        {
            // Remove the item from the timeline
            removeItem( item->track()->trackWorkflow(), item->uuid() );

            // Removing the item from the backend.
            item->track()->trackWorkflow()->removeClip( item->uuid() );
        }
    }
}

void
TracksView::addItem( TrackWorkflow *tw, Workflow::Helper *helper, qint64 start )
{
    Q_ASSERT( helper );

    //If for some reasons the clip was already loaded, don't add it twice.
    //This would likely happen when adding a clip from the timeline, as an element will
    //already be created (by the drag and drop operation)
    if ( m_itemsLoaded.contains( helper->uuid() ) )
        return ;
    qint32                  track = tw->trackId();
    Workflow::TrackType     trackType = tw->type();

    // If there is not enough tracks to insert
    // the clip do it now.
    if ( trackType == Workflow::VideoTrack )
    {
        if ( track + 1 >= m_numVideoTrack )
        {
            int nbTrackToAdd = ( track + 2 ) - m_numVideoTrack;
            for ( int i = 0; i < nbTrackToAdd; ++i )
                addTrack( Workflow::VideoTrack );
        }
    }
    else if ( trackType == Workflow::AudioTrack )
    {
        if ( track + 1 >= m_numAudioTrack )
        {
            int nbTrackToAdd = ( track + 2 ) - m_numAudioTrack;
            for ( int i = 0; i < nbTrackToAdd; ++i )
                addTrack( Workflow::AudioTrack );
        }
    }

    AbstractGraphicsItem        *item = NULL;
    ClipHelper                  *clipHelper = qobject_cast<ClipHelper*>( helper );
    if ( clipHelper != NULL )
    {
        AbstractGraphicsMediaItem   *mediaItem = NULL;
        if ( trackType == Workflow::VideoTrack )
        {
            mediaItem = new GraphicsMovieItem( clipHelper );
            connect( mediaItem, SIGNAL( split(AbstractGraphicsMediaItem*,qint64) ),
                     this, SLOT( split(AbstractGraphicsMediaItem*,qint64) ) );
        }
        else if ( trackType == Workflow::AudioTrack )
        {
            mediaItem = new GraphicsAudioItem( clipHelper );
            connect( mediaItem, SIGNAL( split(AbstractGraphicsMediaItem*,qint64) ),
                     this, SLOT( split(AbstractGraphicsMediaItem*,qint64) ) );
        }
        item = mediaItem;
        m_itemsLoaded.insert( helper->uuid() );
        item->m_tracksView = this;
        item->setHeight( item->itemHeight() );
        item->setTrack( getTrack( trackType, track ) );
        item->setStartPos( start );
        item->m_oldTrack = tw;
        moveItem( item, track, start );
        //If the item has some effects:
        foreach ( EffectHelper *effectHelper, clipHelper->clipWorkflow()->effects( Effect::Filter ) )
        {
            addEffectItem( effectHelper, trackType, track, start );
        }
    }
    else
    {
        EffectHelper    *effectHelper = qobject_cast<EffectHelper*>( helper );
        addEffectItem( effectHelper, trackType, track, start );
    }
    updateDuration();
}

void
TracksView::addEffectItem( EffectHelper *effectHelper, Workflow::TrackType trackType,
                           qint32 trackId, qint64 start )
{
    Q_ASSERT( effectHelper != NULL );
    GraphicsEffectItem *item = new GraphicsEffectItem( effectHelper );
    m_itemsLoaded.insert( effectHelper->uuid() );
    item->m_tracksView = this;
    item->setHeight( item->itemHeight() );
    GraphicsTrack   *track = getTrack( trackType, trackId );
    item->setTrack( track );
    item->setStartPos( start );
    item->m_oldTrack = track->trackWorkflow();
    moveItem( item, trackId, start );
    QList<QGraphicsItem*>     collidingItems = item->collidingItems();
    item->setContainer( NULL );
    foreach ( QGraphicsItem *collider, collidingItems )
    {
        AbstractGraphicsMediaItem   *mediaItem = dynamic_cast<AbstractGraphicsMediaItem*>( collider );
        if ( mediaItem != NULL )
        {
            item->setContainer( mediaItem );
            break ;
        }
    }
}

void
TracksView::dragEnterEvent( QDragEnterEvent *event )
{
    if ( event->mimeData()->hasFormat( "vlmc/uuid" ) )
    {
        event->acceptProposedAction();
        clipDragEnterEvent( event );
    }
    else if ( event->mimeData()->hasFormat( "vlmc/effect_name" ) )
    {
        event->acceptProposedAction();
        effectDragEnterEvent( event );
    }
    else
        event->ignore();
}

void
TracksView::effectDragEnterEvent( QDragEnterEvent *event )
{
    Effect* effect = EffectsEngine::getInstance()->effect( event->mimeData()->data( "vlmc/effect_name") );
    if ( effect != NULL )
    {
        m_dragEffectItem = new GraphicsEffectItem( effect );
        m_dragEffectItem->setHeight( m_dragEffectItem->itemHeight() );
        m_dragEffectItem->m_tracksView = this;
    }
    else
        vlmcWarning() << "Can't find effect name" << event->mimeData()->data( "vlmc/effect_name");
}

void
TracksView::clipDragEnterEvent( QDragEnterEvent *event )
{
    const QString fullId = QString( event->mimeData()->data( "vlmc/uuid" ) );
    Clip *clip = Library::getInstance()->clip( fullId );
    if ( clip == NULL )
        return;
    bool hasVideo = clip->getMedia()->source()->hasVideo();
    bool hasAudio = clip->getMedia()->source()->hasAudio();
    if ( hasAudio == false && hasVideo == false )
        return ;

    if ( hasAudio == true )
    {
        if ( m_dragAudioItem )
            delete m_dragAudioItem;
        m_dragAudioItem = new GraphicsAudioItem( clip );
        m_dragAudioItem->m_tracksView = this;
        m_dragAudioItem->setHeight( m_dragAudioItem->itemHeight() );
        m_dragAudioItem->setTrack( getTrack( m_dragAudioItem->trackType(), 0 ) );
        connect( m_dragAudioItem, SIGNAL( split(AbstractGraphicsMediaItem*,qint64) ),
                 this, SLOT( split(AbstractGraphicsMediaItem*,qint64) ) );
    }
    if ( hasVideo == true )
    {
        if ( m_dragVideoItem )
            delete m_dragVideoItem;
        m_dragVideoItem = new GraphicsMovieItem( clip );
        m_dragVideoItem->m_tracksView = this;
        m_dragVideoItem->setHeight( m_dragVideoItem->itemHeight() );
        m_dragVideoItem->setTrack( getTrack( m_dragVideoItem->trackType(), 0 ) );
        connect( m_dragVideoItem, SIGNAL( split(AbstractGraphicsMediaItem*,qint64) ),
                 this, SLOT( split(AbstractGraphicsMediaItem*,qint64) ) );
    }
    // Group the items together
    if ( hasVideo == true && hasAudio == true )
        m_dragVideoItem->group( m_dragAudioItem );
    if ( hasVideo == false )
        moveItem( m_dragAudioItem, event->pos() );
    else
        moveItem( m_dragVideoItem, event->pos() );
}

void
TracksView::dragMoveEvent( QDragMoveEvent *event )
{
    if ( m_dragVideoItem != NULL )
        moveItem( m_dragVideoItem, event->pos() );
    else if ( m_dragAudioItem != NULL)
        moveItem( m_dragAudioItem, event->pos() );
    else if ( m_dragEffectItem != NULL )
    {
        //Only get medias from here, as we much drag an effect to a media or a track
        QList<AbstractGraphicsMediaItem*>   itemList = mediaItems<AbstractGraphicsMediaItem>( event->pos() );
        if ( itemList.size() > 0 )
        {
            AbstractGraphicsMediaItem   *item = itemList.first();
            ClipHelper                  *clipHelper = qobject_cast<ClipHelper*>( item->helper() );
            Q_ASSERT( clipHelper != NULL );

            m_dragEffectItem->setWidth( item->clipHelper()->length() );
            m_dragEffectItem->setStartPos( item->startPos() );
            m_dragEffectItem->setTrack( item->track() );
            m_dragEffectItem->effectHelper()->setTarget( clipHelper->clipWorkflow() );
        }
        else
        {
            QList<QGraphicsItem*> tracks = items( 0, event->pos().y() );
            foreach ( QGraphicsItem* item, tracks )
            {
                GraphicsTrack   *track = qgraphicsitem_cast<GraphicsTrack*>( item );
                if ( track != NULL && track->mediaType() == Workflow::VideoTrack )
                {
                    m_dragEffectItem->setWidth( m_dragEffectItem->helper()->length() );
                    m_dragEffectItem->setStartPos( 0 );
                    m_dragEffectItem->setTrack( track );
                    m_dragEffectItem->effectHelper()->setTarget( track->trackWorkflow() );
                    break ;
                }
            }
        }
    }
}

void
TracksView::moveItem( TrackWorkflow *tw, const QUuid& uuid, qint64 time )
{
    QList<QGraphicsItem*> sceneItems = m_scene->items();

    for ( int i = 0; i < sceneItems.size(); ++i )
    {
        AbstractGraphicsItem* item =
                dynamic_cast<AbstractGraphicsItem*>( sceneItems.at( i ) );
        if ( !item || item->uuid() != uuid )
            continue;
        moveItem( item, tw->trackId(), time );
        break ;
    }
    updateDuration();
    Timeline::getInstance()->tracksRuler()->update();
}

QPoint
TracksView::boundEffectInClip( GraphicsEffectItem *effectItem, QPoint position )
{
    if ( effectItem->effectHelper()->target()->effectType() == EffectUser::ClipEffectUser )
    {
        QList<QGraphicsItem*>   list = effectItem->collidingItems( Qt::IntersectsItemShape );
        foreach ( QGraphicsItem *graphicsItem, list )
        {
            AbstractGraphicsMediaItem   *mediaItem = dynamic_cast<AbstractGraphicsMediaItem*>( graphicsItem );
            if ( mediaItem == NULL )
                continue ;
            if ( position.x() < mediaItem->pos().x() )
                position.setX( mediaItem->pos().x() );
            if ( position.x() + effectItem->width() > mediaItem->pos().x() + mediaItem->width() )
                position.setX( mediaItem->pos().x() + mediaItem->width() - effectItem->width() );
        }
    }
    return position;
}

void
TracksView::moveItem( AbstractGraphicsItem *item, QPoint position )
{
    GraphicsTrack *track = NULL;

    if ( !m_lastKnownTrack )
        m_lastKnownTrack = getTrack( Workflow::VideoTrack, 0 );

    QPoint  mappedPos = mapToScene( position ).toPoint();

    GraphicsEffectItem  *effectItem = qgraphicsitem_cast<GraphicsEffectItem*>( item );
    if ( effectItem != NULL )
        mappedPos = boundEffectInClip( effectItem, mappedPos );
    QList<QGraphicsItem*> list = items( 0, position.y() );
    for ( int i = 0; i < list.size(); ++i )
    {
        track = qgraphicsitem_cast<GraphicsTrack*>( list.at(i) );
        if ( track )
            break;
    }

    if ( !track )
    {
        // When the mouse pointer is not on a track,
        // use the last known track.
        // This avoids "breaks" when moving a rush
        if ( !m_lastKnownTrack )
            return;
        track = m_lastKnownTrack;
    }

    m_lastKnownTrack = track;

    qreal time = mappedPos.x() + 0.5;
    moveItem( item, track->trackNumber(), (qint64)time);
}

void
TracksView::moveItem( AbstractGraphicsItem *item, qint32 track, qint64 time )
{
    // Add missing tracks
    if ( item->trackType() == Workflow::AudioTrack )
    {
        while ( track >= m_numAudioTrack )
            addTrack( Workflow::AudioTrack );
    }
    else if ( item->trackType() == Workflow::VideoTrack )
    {
        while ( track >= m_numVideoTrack )
            addTrack( Workflow::VideoTrack );
    }

    ItemPosition p = findPosition( item, track, time );

    if ( p.isValid() && item->groupItem() )
    {
        bool validPosFound = false;

        // Add missing tracks for the target
        if ( item->groupItem()->trackType() == Workflow::AudioTrack )
        {
            while ( p.track() >= m_numAudioTrack )
                addTrack( Workflow::AudioTrack );
        }
        else if ( item->groupItem()->trackType() == Workflow::VideoTrack )
        {
            while ( p.track() >= m_numVideoTrack )
                addTrack( Workflow::VideoTrack );
        }

        // Search a position for the linked item
        ItemPosition p2 = findPosition( item->groupItem(), track, time );

        // Add missing tracks for the source
        if ( item->trackType() == Workflow::AudioTrack )
        {
            while ( p2.track() >= m_numAudioTrack )
                addTrack( Workflow::AudioTrack );
        }
        else if ( item->trackType() == Workflow::VideoTrack )
        {
            while ( p2.track() >= m_numVideoTrack )
                addTrack( Workflow::VideoTrack );
        }

        if ( p.time() == p2.time() &&  p.track() == p2.track() )
            validPosFound = true;
        else
        {
            // We did not find a valid position for the two items.
            if ( p.time() == time && p.track() == track )
            {
                // The primary item has found a position that match the request.
                // Ask it to try with the position of the linked item.
                p = findPosition( item, p2.track(), p2.time() );

                if ( p.time() == p2.time() && p.track() == p2.track() )
                    validPosFound = true;
            }
            else if ( p2.time() == time && p2.track() == track )
            {
                // The linked item has found a position that match the request.
                // Ask it to try with the position of the primary item.
                p2 = findPosition( item->groupItem(), p.track(), p.time() );

                if ( p.time() == p2.time() && p.track() == p2.track() )
                    validPosFound = true;
            }
        }

        if ( validPosFound )
        {
            // We've found a valid position that fit for the two items.
            // Move the primary item to the target destination.
            item->setStartPos( p.time() );
            item->setTrack( getTrack( item->trackType(), p.track() ) );

            // Move the linked item to the target destination.
            item->groupItem()->setStartPos( p2.time() );
            item->groupItem()->setTrack( getTrack( item->groupItem()->trackType(), p2.track() ) );
        }
    }
    else
    {
        if ( p.isValid() )
        {
            item->setStartPos( p.time() );
            item->setTrack( getTrack( item->trackType(), p.track() ) );
        }
    }
}

ItemPosition
TracksView::findPosition( AbstractGraphicsItem *item, qint32 track, qint64 time )
{
    if ( qgraphicsitem_cast<GraphicsEffectItem*>( item ) != NULL )
        return ItemPosition( track, time );
    // Create a fake item for computing collisions
    QGraphicsRectItem *chkItem = new QGraphicsRectItem( item->boundingRect().adjusted( 0, 1, 0, -1 ) );
    chkItem->setParentItem( getTrack( item->trackType(), track ) );
    chkItem->setPos( time, 0 );

    QGraphicsItem *oldParent = item->parentItem();
    qreal oldPos = item->startPos();

    // Check for vertical collisions
    bool continueSearch = true;
    while ( continueSearch )
    {
        QList<QGraphicsItem*> colliding = chkItem->collidingItems( Qt::IntersectsItemShape );
        bool itemCollision = false;
        for ( int i = 0; i < colliding.size(); ++i )
        {
            AbstractGraphicsMediaItem *currentItem = dynamic_cast<AbstractGraphicsMediaItem*>( colliding.at( i ) );
            if ( currentItem && currentItem != item )
            {
                qint32  trackId = currentItem->trackNumber();
                Q_ASSERT( trackId >= 0 );

                // Collision with an item of the same type
                itemCollision = true;
                if ( trackId > track )
                {
                    if ( track < 1 )
                    {
                        chkItem->setParentItem( oldParent );
                        continueSearch = false;
                        break;
                    }
                    track -= 1;
                }
                else if ( trackId <= track )
                {
                    int higherTrack = 0;
                    if ( item->trackType() == Workflow::VideoTrack )
                        higherTrack = m_numVideoTrack;
                    else if ( item->trackType() == Workflow::AudioTrack )
                        higherTrack = m_numAudioTrack;

                    if ( track >= higherTrack - 1 )
                    {
                        chkItem->setParentItem( oldParent );
                        continueSearch = false;
                        break;
                    }
                    track += 1;
                }
                Q_ASSERT( getTrack( item->trackType(), track ) != NULL );
                chkItem->setParentItem( getTrack( item->trackType(), track ) );
            }
        }
        if ( !itemCollision )
            continueSearch = false;
    }


    // Check for horizontal collisions
    chkItem->setPos( qMax( time, (qint64)0 ), 0 );

    AbstractGraphicsMediaItem *hItem = NULL;
    QList<QGraphicsItem*> collide = chkItem->collidingItems( Qt::IntersectsItemShape );
    for ( int i = 0; i < collide.count(); ++i )
    {
        hItem = dynamic_cast<AbstractGraphicsMediaItem*>( collide.at( i ) );
        if ( hItem && hItem != item ) break;
    }

    if ( hItem && hItem != item )
    {
        qreal newpos;
        // Evaluate a possible solution
        if ( chkItem->pos().x() > hItem->pos().x() )
            newpos = hItem->pos().x() + hItem->boundingRect().width();
        else
            newpos = hItem->pos().x() - chkItem->boundingRect().width();

        if ( newpos < 0 || newpos == hItem->pos().x() )
            chkItem->setPos( oldPos, 0 ); // Fail
        else
        {
            // A solution may be found
            chkItem->setPos( qRound64( newpos ), 0 );
            QList<QGraphicsItem*> collideAgain = chkItem->collidingItems( Qt::IntersectsItemShape );
            for ( int i = 0; i < collideAgain.count(); ++i )
            {
                AbstractGraphicsMediaItem *currentItem =
                        dynamic_cast<AbstractGraphicsMediaItem*>( collideAgain.at( i ) );
                if ( currentItem && currentItem != item )
                {
                    chkItem->setPos( oldPos, 0 ); // Fail
                    break;
                }
            }
        }
    }

    GraphicsTrack *t = static_cast<GraphicsTrack*>( chkItem->parentItem() );

    ItemPosition p;
    p.setTime( chkItem->pos().x() );

    if ( t )
        p.setTrack( t->trackNumber() );
    else
        p.setTrack( -1 ); // Return in valid position

    delete chkItem;
    return p;
}

void
TracksView::removeItem( TrackWorkflow *tw, const QUuid &uuid )
{
    GraphicsTrack           *track = getTrack( tw->type(), tw->trackId() );

    if ( track == NULL )
        return ;
    QList<QGraphicsItem*> trackItems = track->childItems();;

    for ( int i = 0; i < trackItems.size(); ++i )
    {
        AbstractGraphicsItem    *item = dynamic_cast<AbstractGraphicsItem*>( trackItems.at( i ) );
        if ( !item || item->uuid() != uuid )
            continue;
        removeItem( item );
    }
}

void
TracksView::removeItem( AbstractGraphicsItem *item )
{
    // Is it the same item captured by mouse events
    if( item == m_actionItem )
        m_actionItem = NULL;

    m_itemsLoaded.remove( item->uuid() );
    delete item;
    updateDuration();
}

void
TracksView::dragLeaveEvent( QDragLeaveEvent *event )
{
    Q_UNUSED( event )
    bool updateDurationNeeded = false;
    if ( m_dragAudioItem || m_dragVideoItem )
        updateDurationNeeded = true;

    delete m_dragAudioItem;
    delete m_dragVideoItem;
    delete m_dragEffectItem;
    m_dragAudioItem = NULL;
    m_dragVideoItem = NULL;
    m_dragEffectItem = NULL;

    if ( updateDurationNeeded )
        updateDuration();
}

void
TracksView::dropEvent( QDropEvent *event )
{
    qreal mappedXPos = ( mapToScene( event->pos() ).x() + 0.5 );;

    if ( m_dragAudioItem != NULL || m_dragVideoItem != NULL )
    {
        UndoStack::getInstance()->beginMacro( "Add clip" );

        if ( m_dragAudioItem )
        {
            m_itemsLoaded.insert( m_dragAudioItem->clipHelper()->uuid() );

            updateDuration();
            if ( getTrack( Workflow::AudioTrack, m_numAudioTrack - 1 )->childItems().count() > 0 )
                addTrack( Workflow::AudioTrack );
            event->acceptProposedAction();

            m_dragAudioItem->m_oldTrack = m_dragAudioItem->track()->trackWorkflow();

            Commands::trigger( new Commands::Clip::Add( m_dragAudioItem->clipHelper(),
                                                                    m_dragAudioItem->track()->trackWorkflow(),
                                                                    (qint64)mappedXPos ) );
            m_dragAudioItem = NULL;
        }

        if ( m_dragVideoItem )
        {
            m_itemsLoaded.insert( m_dragVideoItem->clipHelper()->uuid() );

            updateDuration();
            if ( getTrack( Workflow::VideoTrack, m_numVideoTrack - 1 )->childItems().count() > 0 )
                addTrack( Workflow::VideoTrack );
            event->acceptProposedAction();

            m_dragVideoItem->m_oldTrack = m_dragVideoItem->track()->trackWorkflow();

            Commands::trigger( new Commands::Clip::Add( m_dragVideoItem->clipHelper(),
                                                                    m_dragVideoItem->track()->trackWorkflow(),
                                                                    (qint64)mappedXPos ) );
            m_dragVideoItem = NULL;
        }

        UndoStack::getInstance()->endMacro();

        m_lastKnownTrack = NULL;
    }
    else if ( m_dragEffectItem != NULL )
    {
        QList<AbstractGraphicsMediaItem*>   clips = mediaItems<AbstractGraphicsMediaItem>( event->pos() );
        if ( clips.size() > 0 )
        {
            m_itemsLoaded.insert( m_dragEffectItem->helper()->uuid() );
            AbstractGraphicsMediaItem   *item = clips.first();
            Commands::trigger( new Commands::Effect::Add( m_dragEffectItem->effectHelper(),
                                                          item->clipHelper()->clipWorkflow() ) );
            m_dragEffectItem->m_oldTrack = item->track()->trackWorkflow();
            event->acceptProposedAction();
            m_dragEffectItem->setContainer( item );
        }
        else
        {
            QList<QGraphicsItem*> tracks = items( 0, event->pos().y() );
            foreach ( QGraphicsItem* item, tracks )
            {
                GraphicsTrack   *track = qgraphicsitem_cast<GraphicsTrack*>( item );
                if ( track != NULL )
                {
                    m_itemsLoaded.insert( m_dragEffectItem->helper()->uuid() );
                    updateDuration();
                    if ( getTrack( Workflow::VideoTrack, m_numVideoTrack - 1 )->childItems().count() > 0 )
                        addTrack( Workflow::VideoTrack );
                    m_dragEffectItem->m_oldTrack = track->trackWorkflow();
                    Commands::trigger( new Commands::Effect::Add( m_dragEffectItem->effectHelper(),
                                                                  track->trackWorkflow() ) );

                    event->acceptProposedAction();
                    m_dragEffectItem->setContainer( NULL );
                    break ;
                }
            }
        }
    }
}

void
TracksView::setDuration( int duration )
{
    int diff = ( int ) qAbs( ( qreal )duration - sceneRect().width() );
    if ( diff * matrix().m11() > -50 )
    {
        if ( matrix().m11() < 0.4 )
            setSceneRect( 0, 0, ( duration + 100 / matrix().m11() ), sceneRect().height() );
        else
            setSceneRect( 0, 0, ( duration + 300 ), sceneRect().height() );
    }
    m_projectDuration = duration;
}

void
TracksView::setTool( ToolButtons button )
{
    m_tool = button;
    if ( m_tool == TOOL_CUT )
        scene()->clearSelection();
}

void
TracksView::resizeEvent( QResizeEvent *event )
{
    QGraphicsView::resizeEvent( event );
}

void
TracksView::drawBackground( QPainter *painter, const QRectF &rect )
{
    // Fill the background
    painter->fillRect( rect, QBrush( palette().base() ) );

    // Draw the tracks separators
    painter->setPen( QPen( QColor( 72, 72, 72 ) ) );
    for ( int i = 0; i < m_layout->count(); ++i )
    {
        QGraphicsItem* gi = m_layout->itemAt( i )->graphicsItem();
        if ( !gi ) continue;
        GraphicsTrack* track = qgraphicsitem_cast<GraphicsTrack*>( gi );
        if ( !track ) continue;

        QRectF trackRect = track->mapRectToScene( track->boundingRect() );
        if ( track->mediaType() == Workflow::VideoTrack )
            painter->drawLine( rect.left(), trackRect.top(), rect.right() + 1, trackRect.top() );
        else
            painter->drawLine( rect.left(), trackRect.bottom(), rect.right() + 1, trackRect.bottom() );
    }

    // Audio/Video separator
    QLinearGradient g( 0, m_separator->y(), 0, m_separator->y() + m_separator->boundingRect().height() );
    QColor base = palette().window().color();
    QColor end = palette().dark().color();
    g.setColorAt( 0, end );
    g.setColorAt( 0.1, base );
    g.setColorAt( 0.9, base );
    g.setColorAt( 1.0, end );

    painter->setBrush( QBrush( g ) );
    painter->setPen( Qt::transparent );
    painter->drawRect( rect.left(),
                       (int) m_separator->y(),
                       (int) rect.right() + 1,
                       (int) m_separator->boundingRect().height() );

}

void
TracksView::mouseMoveEvent( QMouseEvent *event )
{
    if ( event->modifiers() == Qt::NoModifier &&
         event->buttons() == Qt::LeftButton &&
         m_action == TracksView::Move )
    {
        // Check if the item exists or has been removed
        if ( m_actionItem == NULL )
            return;

        //Moving item.
        m_actionItem->setOpacity( 0.6F );
        if ( m_actionRelativeX < 0 )
            m_actionRelativeX = event->pos().x() - mapFromScene( m_actionItem->pos() ).x();
        QPoint  pos( event->pos().x() - m_actionRelativeX, event->pos().y() );
        moveItem( m_actionItem, pos );
        GraphicsEffectItem  *effectItem = qgraphicsitem_cast<GraphicsEffectItem*>( m_actionItem );
        if ( effectItem != NULL )
        {
            QList<QGraphicsItem*>   list = m_actionItem->collidingItems();
            m_effectTarget = NULL;
            foreach ( QGraphicsItem *collidingItem, list )
            {
                AbstractGraphicsMediaItem   *mediaItem = dynamic_cast<AbstractGraphicsMediaItem*>( collidingItem );
                if ( mediaItem != NULL )
                {
                    ClipHelper  *clipHelper = qobject_cast<ClipHelper*>( mediaItem->helper() );
                    Q_ASSERT( clipHelper != NULL );
                    m_effectTarget = mediaItem;
//                    effectItem->effectHelper()->setTarget( clipHelper->clipWorkflow() );
                    break ;
                }
            }
//            if ( m_effectTarget == NULL ) //Avoid doing this all the time.
//            {
//                GraphicsTrack *track = getTrack( m_actionItem->trackType(), m_actionItem->trackNumber() );
//                effectItem->effectHelper()->setTarget( track->trackWorkflow() );
//            }
        }
    }
    else if ( event->modifiers() == Qt::NoModifier &&
              event->buttons() == Qt::LeftButton &&
              m_action == TracksView::Resize )
    {
        qint64  itemPos = m_actionItem->mapToScene( 0, 0 ).x();
        qint64  itemNewSize;
        if ( m_actionResizeType == AbstractGraphicsItem::BEGINNING )
            itemNewSize = mapToScene( event->pos() ).x() - itemPos;
        else
            itemNewSize = itemPos + m_actionItem->width() - mapToScene( event->pos() ).x();

        qint32  trackId = m_actionItem->trackNumber();
        Q_ASSERT( trackId >= 0 );

        //FIXME: BEGIN UGLY
        GraphicsTrack *track = getTrack( m_actionItem->trackType(), trackId );
        Q_ASSERT( track );

        QPointF collidePos = track->sceneBoundingRect().topRight();
        collidePos.setX( itemPos + itemNewSize );

        QList<QGraphicsItem*> gi = scene()->items( collidePos );

        bool collide = false;
        if ( m_actionItem->type() != GraphicsEffectItem::Type )
        {
            for ( int i = 0; i < gi.count(); ++i )
            {
                AbstractGraphicsMediaItem* mi = dynamic_cast<AbstractGraphicsMediaItem*>( gi.at( i ) );
                if ( mi && mi != m_actionItem )
                {
                    collide = true;
                    break;
                }
            }
        }
        // END UGLY
        if ( !collide )
        {
            GraphicsEffectItem  *effectItem = qgraphicsitem_cast<GraphicsEffectItem*>( m_actionItem );
            if ( effectItem != NULL )
            {
                QList<QGraphicsItem*>   list = m_actionItem->collidingItems();
                foreach ( QGraphicsItem *collidingItem, list )
                {
                    AbstractGraphicsMediaItem   *mediaItem = dynamic_cast<AbstractGraphicsMediaItem*>( collidingItem );
                    if ( mediaItem != NULL )
                    {
                        m_effectTarget = mediaItem;
                        break ;
                    }
                }
            }
            if ( m_actionResizeType == AbstractGraphicsItem::END )
            {
                m_actionItem->resize( itemNewSize, mapToScene( event->pos() ).x(),
                                      itemPos + itemNewSize, AbstractGraphicsItem::END );
            }
            else
            {
                m_actionItem->resize( itemNewSize, itemPos + itemNewSize,
                                      itemPos, AbstractGraphicsItem::BEGINNING );
            }
        }
    }

    QGraphicsView::mouseMoveEvent( event );
}

void
TracksView::mousePressEvent( QMouseEvent *event )
{
    QList<AbstractGraphicsItem*> mediaCollisionList = mediaItems<AbstractGraphicsItem>( event->pos() );

    // Reset the drag mode
    setDragMode( QGraphicsView::NoDrag );

    if ( event->modifiers() == Qt::ControlModifier && mediaCollisionList.count() == 0 )
    {
        setDragMode( QGraphicsView::ScrollHandDrag );
        event->accept();
    }
    else if ( event->modifiers() == Qt::NoModifier &&
         event->button() == Qt::LeftButton &&
         tool() == TOOL_DEFAULT &&
         mediaCollisionList.count() >= 1 )
    {
        AbstractGraphicsItem *item = mediaCollisionList.at( 0 );

        QPoint itemEndPos = mapFromScene( item->mapToScene( item->boundingRect().bottomRight() ) );
        QPoint itemPos = mapFromScene( item->mapToScene( 0, 0 ) );
        QPoint clickPos = event->pos() - itemPos;
        QPoint itemSize = itemEndPos - itemPos;

        if ( clickPos.x() < RESIZE_ZONE || clickPos.x() > ( itemSize.x() - RESIZE_ZONE ) )
        {
            if ( clickPos.x() < RESIZE_ZONE )
                m_actionResizeType = AbstractGraphicsItem::END;
            else
                m_actionResizeType = AbstractGraphicsItem::BEGINNING;
            m_action = TracksView::Resize;
            m_actionItem = item;
        }
        else if ( item->moveable() )
        {
            m_action = Move;
            m_actionItem = item;
            m_effectTarget = NULL;
        }
        scene()->clearSelection();
        item->setSelected( true );
        event->accept();
    }
    else if ( event->modifiers() == Qt::NoModifier &&
         event->button() == Qt::RightButton &&
         tool() == TOOL_DEFAULT &&
         mediaCollisionList.count() == 1 )
    {
        AbstractGraphicsItem    *item = mediaCollisionList.at( 0 );

        if ( !scene()->selectedItems().contains( item ) )
        {
            scene()->clearSelection();
            item->setSelected( true );
        }
    }
    else if ( event->modifiers() == Qt::ControlModifier &&
              event->button() == Qt::LeftButton &&
              tool() == TOOL_DEFAULT &&
              mediaCollisionList.count() == 1 )
    {
        AbstractGraphicsItem        *item = mediaCollisionList.at( 0 );
        item->setSelected( !item->isSelected() );
        event->accept();
    }
    else if ( event->modifiers() & Qt::ShiftModifier && mediaCollisionList.count() == 0 )
    {
        setDragMode( QGraphicsView::RubberBandDrag );
        if ( !event->modifiers() & Qt::ControlModifier )
            scene()->clearSelection();
        event->accept();
    }

    QGraphicsView::mousePressEvent( event );
}

void
TracksView::mouseReleaseEvent( QMouseEvent *event )
{
    if ( m_action == TracksView::Move )
    {
        // Check if the item exists or has been removed
        if ( m_actionItem == NULL )
            return;

        m_actionItem->setOpacity( 1.0F );
        GraphicsEffectItem  *effectItem = qgraphicsitem_cast<GraphicsEffectItem*>( m_actionItem );

        //Check if the item moved.
        if ( effectItem != NULL )
        {
            if ( m_actionItem->m_oldTrack == m_actionItem->track()->trackWorkflow() )
            {
                const AbstractGraphicsMediaItem *container = effectItem->container();
                if ( container != NULL && container->startPos() + effectItem->begin() == effectItem->startPos() )
                    goto out;
                else if ( container == NULL && effectItem->startPos() == effectItem->begin() )
                    goto out;
            }
        }
        else
        {
            if ( m_actionItem->m_oldTrack == m_actionItem->track()->trackWorkflow() &&
                 m_actionItem->startPos() == m_actionItem->track()->trackWorkflow()->getClipPosition( m_actionItem->uuid() ) )
                goto out;
        }

        updateDuration();

        if ( getTrack( Workflow::VideoTrack, m_numVideoTrack - 1 )->childItems().count() > 0 )
            addTrack( Workflow::VideoTrack );
        if ( getTrack( Workflow::AudioTrack, m_numAudioTrack - 1 )->childItems().count() > 0 )
            addTrack( Workflow::AudioTrack );

        EffectUser  *target = m_actionItem->track()->trackWorkflow();
        qint64      targetPos = m_actionItem->startPos();
        if ( effectItem != NULL )
        {
            effectItem->setContainer( NULL );
            if ( m_effectTarget != NULL )
            {
                target = m_effectTarget->clipHelper()->clipWorkflow();
                targetPos = m_actionItem->startPos() - m_effectTarget->startPos();
                effectItem->setContainer( m_effectTarget );
            }
        }
        else
            UndoStack::getInstance()->beginMacro( "Move clip" );
        m_actionItem->triggerMove( target, targetPos );
        // Update the linked item too
        if ( m_actionItem->groupItem() )
        {
            m_actionItem->groupItem()->triggerMove( m_actionItem->groupItem()->track()->trackWorkflow(),
                                                    m_actionItem->groupItem()->startPos() );
            m_actionItem->groupItem()->m_oldTrack = m_actionItem->groupItem()->track()->trackWorkflow();
        }

        if ( effectItem == NULL )
            UndoStack::getInstance()->endMacro();

        m_actionItem->m_oldTrack = m_actionItem->track()->trackWorkflow();
        m_actionRelativeX = -1;
        m_lastKnownTrack = NULL;
    }
    else if ( m_action == TracksView::Resize )
    {
        qint64  newBegin;
        qint64  newEnd;
        if ( m_actionResizeType == AbstractGraphicsItem::END )
        {
            newEnd = m_actionItem->helper()->end();
            newBegin = newEnd - m_actionItem->width();
        }
        else
        {
            newBegin = m_actionItem->helper()->begin();
            newEnd = newBegin + m_actionItem->width();
        }
        EffectUser          *target = m_actionItem->track()->trackWorkflow();
        GraphicsEffectItem  *effectItem = qgraphicsitem_cast<GraphicsEffectItem*>( m_actionItem );
        if ( effectItem != NULL && m_effectTarget != NULL )
            target = m_effectTarget->clipHelper()->clipWorkflow();
        m_actionItem->triggerResize( target, m_actionItem->helper(),
                                     newBegin, newEnd, m_actionItem->pos().x() );
        updateDuration();
    }

out:
    m_effectTarget = NULL;
    m_actionItem = NULL;
    m_action = TracksView::None;
    //setDragMode( QGraphicsView::NoDrag );
    QGraphicsView::mouseReleaseEvent( event );
}

void
TracksView::wheelEvent( QWheelEvent *event )
{
    if ( event->modifiers() == Qt::ControlModifier )
    {
        // CTRL + WHEEL = Zoom
        if ( event->delta() > 0 )
            emit zoomIn();
        else
            emit zoomOut();
        event->accept();
    }
    else
    {
        //TODO should scroll the timeline
        event->ignore();
        QGraphicsView::wheelEvent( event );
    }
}

QList<AbstractGraphicsItem*>
TracksView::timelineItems()
{
    //TODO optimization needed!
    QGraphicsItem *item;
    QList<AbstractGraphicsItem*> outlist;
    QList<QGraphicsItem*> list = items();
    foreach( item, list )
    {
        AbstractGraphicsItem *ami = dynamic_cast<AbstractGraphicsItem*>( item );
        if ( ami != NULL )
            outlist.append( ami );
    }
    return outlist;
}

void
TracksView::setCursorPos( qint64 pos )
{
    if ( pos < 0 )
        pos = 0;
    m_cursorLine->frameChanged( pos, Vlmc::TimelineCursor );
}

qint64
TracksView::cursorPos()
{
    return m_cursorLine->cursorPos();
}

void
TracksView::setScale( double scaleFactor )
{
    QMatrix matrix;
    matrix.scale( scaleFactor, 1 );
    //TODO update the scene scale?
    setMatrix( matrix );

    int diff = ( int ) ( sceneRect().width() - ( qreal ) m_projectDuration );
    if ( diff * matrix.m11() < 50 )
    {
        if ( matrix.m11() < 0.4 )
            setSceneRect( 0, 0, ( m_projectDuration + 100 / matrix.m11() ), sceneRect().height() );
        else
            setSceneRect( 0, 0, ( m_projectDuration + 300 ), sceneRect().height() );
    }
    centerOn( m_cursorLine );
}

void
TracksView::ensureCursorVisible()
{
    if ( horizontalScrollBar()->isVisible() )
    {
        QRectF visibleArea = visibleRect();
        ensureVisible( cursorPos(),
                       visibleArea.y() + ( visibleArea.height() / 2 ),
                       1, 1, 150, 0 );
    }
}

QRectF
TracksView::visibleRect()
{
    QPointF topLeft( horizontalScrollBar()->value(), verticalScrollBar()->value() );
    QPointF bottomRight( topLeft + viewport()->rect().bottomRight() );
    QMatrix reverted = matrix().inverted();
    return reverted.mapRect( QRectF( topLeft, bottomRight ) );
}

void
TracksView::updateDuration()
{
    //TODO this should use a variant of mediaItems( const QPoint& )
    QList<QGraphicsItem*> sceneItems = m_scene->items();

    int projectDuration = 0;
    for ( int i = 0; i < sceneItems.size(); ++i )
    {
        AbstractGraphicsMediaItem *item =
                dynamic_cast<AbstractGraphicsMediaItem*>( sceneItems.at( i ) );
        if ( !item ) continue;
        if ( ( item->startPos() + item->boundingRect().width() ) > projectDuration )
            projectDuration = ( int ) ( item->startPos() + item->boundingRect().width() );
    }

    m_projectDuration = projectDuration;

    // Make sure that the width is not below zero
    int minimumWidth = qMax( m_projectDuration, 0 );

    // PreferredWidth not working?
    m_layout->setMinimumWidth( minimumWidth );
    m_layout->setMaximumWidth( minimumWidth );

    setSceneRect( m_layout->contentsRect() );

    emit durationChanged( m_projectDuration );

    // Also check for unused tracks
    cleanUnusedTracks();
}

void
TracksView::cleanTracks( Workflow::TrackType type )
{
    int tracksToCheck;
    int tracksToRemove = 0;

    if ( type == Workflow::VideoTrack )
        tracksToCheck = m_numVideoTrack;
    else
        tracksToCheck = m_numAudioTrack;

    for ( int i = tracksToCheck; i > 0; --i )
    {
        GraphicsTrack *track = getTrack( type, i );
        if ( !track )
            continue;

        QList<AbstractGraphicsItem*> items = track->childs();

        if ( items.count() == 0 )
            tracksToRemove++;
        else
            break;
    }

    while ( tracksToRemove > 1 )
    {
        if ( type == Workflow::VideoTrack )
            removeVideoTrack();
        else
            removeAudioTrack();
        tracksToRemove--;
    }
}

void
TracksView::cleanUnusedTracks()
{
    // Video
    cleanTracks( Workflow::VideoTrack );
    // Audio
    cleanTracks( Workflow::AudioTrack );
}

GraphicsTrack*
TracksView::getTrack( Workflow::TrackType type, unsigned int number )
{
    for (int i = 0; i < m_layout->count(); ++i )
    {
        QGraphicsItem *gi = m_layout->itemAt( i )->graphicsItem();
        GraphicsTrack *track = qgraphicsitem_cast<GraphicsTrack*>( gi );
        if ( !track ) continue;
        if ( track->mediaType() != type ) continue;
        if ( track->trackNumber() == number )
            return track;
    }
    return NULL;
}

void
TracksView::split( AbstractGraphicsMediaItem *item, qint64 frame )
{
    //frame is the number of frame from the beginning of the clip
    //item->startPos() is the position of the splitted clip (in frame)
    //therefore, the position of the newly created clip is
    //the splitted clip pos + the splitting point (ie startPos() + frame)
    Commands::trigger( new Commands::Clip::Split( item->track()->trackWorkflow(),
                                            item->clipHelper(), item->startPos() + frame,
                                            frame + item->clipHelper()->begin() ) );
}

AbstractGraphicsMediaItem*
TracksView::item( const QUuid &uuid )
{
    for ( int i = 0; i < m_scene->items().size(); ++i )
    {
        AbstractGraphicsMediaItem* item =
                dynamic_cast<AbstractGraphicsMediaItem*>( m_scene->items().at( i ) );
        if ( item == NULL )
            continue ;
        if ( item->uuid() == uuid )
            return item;
    }
    return NULL;
}

TracksView::Action
TracksView::currentAction() const
{
    return m_action;
}
