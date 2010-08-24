/*****************************************************************************
 * Timeline.cpp: Widget that handle the tracks
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

#include "Timeline.h"

#include "Clip.h"
#include "ClipHelper.h"
#include "TracksView.h"
#include "TracksScene.h"
#include "TracksControls.h"
#include "TracksRuler.h"
#include "WorkflowRenderer.h"

#include <QHBoxLayout>
#include <QDomElement>
#include <QScrollBar>

#include <QtDebug>

Timeline*   Timeline::m_instance = NULL;

Timeline::Timeline( WorkflowRenderer* renderer, QWidget *parent ) :
    QWidget( parent ), m_scale( 1.0 ), m_renderer( renderer )
{
    Q_ASSERT( m_instance == NULL );
    m_instance = this;
    m_ui.setupUi( this );

    m_mainWorkflow = MainWorkflow::getInstance();

    m_tracksScene = new TracksScene( this );
    m_tracksView = new TracksView( m_tracksScene, m_mainWorkflow, m_renderer, m_ui.tracksFrame );
    m_tracksView->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    m_tracksView->scale(1, 1);

    QHBoxLayout* tracksViewLayout = new QHBoxLayout();
    tracksViewLayout->setContentsMargins( 0, 0, 0, 0 );
    m_ui.tracksFrame->setLayout( tracksViewLayout );
    tracksViewLayout->addWidget( m_tracksView );

    m_tracksRuler = new TracksRuler( tracksView(), this );
    QHBoxLayout* tracksRulerLayout = new QHBoxLayout();
    tracksRulerLayout->setContentsMargins( 0, 0, 0, 0 );
    m_ui.rulerFrame->setLayout( tracksRulerLayout );
    tracksRulerLayout->addWidget( m_tracksRuler );

    m_tracksControls = new TracksControls( this );
    QHBoxLayout* tracksControlsLayout = new QHBoxLayout();
    tracksControlsLayout->setContentsMargins( 0, 0, 0, 0 );
    m_ui.controlsFrame->setLayout( tracksControlsLayout );
    tracksControlsLayout->addWidget( m_tracksControls );


    changeZoom( 10 );
    setDuration( 0 );

    // Scroll
    connect( m_tracksView->horizontalScrollBar(), SIGNAL( valueChanged(int) ),
             m_tracksRuler, SLOT( moveRuler(int) ) );
    connect( m_tracksView->verticalScrollBar(), SIGNAL( valueChanged(int) ),
             m_tracksControls->verticalScrollBar(), SLOT( setValue(int) ) );
    connect( m_tracksControls->verticalScrollBar(), SIGNAL( valueChanged(int) ),
             m_tracksView->verticalScrollBar(), SLOT( setValue(int) ) );

    // Project duration change
    connect( m_tracksView, SIGNAL( durationChanged(int) ), this, SLOT( setDuration(int) ) );

    // Clip actions
    connect( m_mainWorkflow, SIGNAL( clipAdded( ClipHelper*, uint, qint64, Workflow::TrackType ) ),
             this, SLOT( actionAddClip(ClipHelper*, uint, qint64, Workflow::TrackType ) ) );
    connect( m_mainWorkflow, SIGNAL( clipMoved( QUuid, uint, qint64, Workflow::TrackType ) ),
             this, SLOT( actionMoveClip(QUuid, uint, qint64, Workflow::TrackType ) ) );
    connect( m_mainWorkflow, SIGNAL( clipRemoved( const QUuid&, uint, Workflow::TrackType ) ),
             this, SLOT( actionRemoveClip( const QUuid&, uint, Workflow::TrackType )) );

    // Clear / reset
    connect( m_mainWorkflow, SIGNAL( cleared() ), m_tracksControls, SLOT( clear() ) );
    connect( m_mainWorkflow, SIGNAL( cleared() ), tracksView(), SLOT( clear() ) );

    // Tracks controls
    connect( m_tracksView, SIGNAL( videoTrackAdded(GraphicsTrack*) ),
             m_tracksControls, SLOT( addVideoTrack(GraphicsTrack*) ) );
    connect( m_tracksView, SIGNAL( audioTrackAdded(GraphicsTrack*) ),
             m_tracksControls, SLOT( addAudioTrack(GraphicsTrack*) ) );
    connect( m_tracksView, SIGNAL( videoTrackRemoved() ),
             m_tracksControls, SLOT( removeVideoTrack() ) );
    connect( m_tracksView, SIGNAL( audioTrackRemoved() ),
             m_tracksControls, SLOT( removeAudioTrack() ) );

    // Frames updates
    connect( m_renderer, SIGNAL( frameChanged(qint64, Vlmc::FrameChangedReason) ),
             m_tracksView->tracksCursor(), SLOT( frameChanged( qint64, Vlmc::FrameChangedReason ) ),
             Qt::QueuedConnection );
    connect( m_renderer, SIGNAL( frameChanged(qint64,Vlmc::FrameChangedReason) ),
             m_tracksRuler, SLOT( update() ) );
    connect( m_tracksRuler, SIGNAL( frameChanged(qint64,Vlmc::FrameChangedReason) ),
             m_renderer, SLOT( rulerCursorChanged(qint64)) );

    // Cursor position updates
    connect( m_tracksView->tracksCursor(), SIGNAL( cursorPositionChanged( qint64 ) ),
             m_renderer, SLOT( timelineCursorChanged(qint64) ) );

    m_tracksView->createLayout();
}

Timeline::~Timeline()
{
    MainWorkflow::destroyInstance();
}

void Timeline::changeEvent( QEvent *e )
{
    switch ( e->type() )
    {
    case QEvent::LanguageChange:
        m_ui.retranslateUi( this );
        break;
    default:
        break;
    }
}

void Timeline::clear()
{
    // The main workflow will ask the GUI to clear itself.
    m_mainWorkflow->clear();
}

void Timeline::changeZoom( int factor )
{
    m_tracksRuler->setPixelPerMark( factor );
    m_scale = (double) FRAME_SIZE / m_tracksRuler->comboScale[factor];
    m_tracksView->setScale( m_scale );
}

void Timeline::setDuration( int duration )
{
    m_tracksView->setDuration( duration );
    m_tracksRuler->setDuration( duration );
}

void Timeline::setTool( ToolButtons button )
{
    tracksView()->setTool( button );
}

void Timeline::actionAddClip( ClipHelper* clipHelper, unsigned int track, qint64 start, Workflow::TrackType trackType )
{
    tracksView()->addMediaItem( clipHelper, track, trackType, start );
}

void Timeline::actionMoveClip( const QUuid& uuid, unsigned int track, qint64 time, Workflow::TrackType )
{
    tracksView()->moveMediaItem( uuid, track, time );
    Q_ASSERT( tracksView()->setItemOldTrack( uuid, track ) == true );
    tracksView()->updateDuration();
    tracksRuler()->update();
}

void Timeline::actionRemoveClip( const QUuid &uuid, unsigned int track, Workflow::TrackType trackType )
{
    tracksView()->removeMediaItem( uuid, track, trackType );
}

void
Timeline::save( QXmlStreamWriter &project ) const
{
    project.writeStartElement( "timeline" );
    for ( int i = 0; i < tracksView()->m_scene->items().size(); ++i )
    {
        AbstractGraphicsMediaItem* item =
                dynamic_cast<AbstractGraphicsMediaItem*>( tracksView()->m_scene->items().at( i ) );

        if ( item == NULL )
            continue ;
        project.writeStartElement( "item" );
        project.writeAttribute( "uuid", item->clipHelper()->uuid().toString() );
        if ( item->itemColor().isValid() == true )
            project.writeAttribute( "color", item->itemColor().name() );
        if ( item->groupItem() != NULL )
        {
            project.writeStartElement( "linkedTo" );
                project.writeStartElement( "item" );
                    project.writeAttribute( "uuid", item->groupItem()->uuid() );
                project.writeEndElement();
            project.writeEndElement();
        }
        project.writeEndElement();
    }
    project.writeEndDocument();
}

void
Timeline::load( const QDomElement &root )
{
    QDomElement     project = root.firstChildElement( "timeline" );
    if ( project.isNull() == true )
    {
        qCritical() << "No timeline node in the project file";
        return ;
    }

    QDomElement elem = project.firstChildElement();
    while ( elem.isNull() == false )
    {
        QString uuid = elem.attribute( "uuid" );

        AbstractGraphicsMediaItem   *item = tracksView()->item( uuid );
        if ( item != NULL )
        {
            if ( elem.hasAttribute( "color" ) == true )
            {
                QString     color = elem.attribute( "color" );
                item->setColor( color );
            }
            QDomElement links = elem.firstChildElement( "linkedTo" );
            if ( links.isNull() == false )
            {
                QDomElement linkedItem = links.firstChildElement();
                while ( linkedItem.isNull() == false )
                {
                    QString     linkedUuid = linkedItem.attribute( "uuid" );
                    AbstractGraphicsMediaItem   *li = tracksView()->item( linkedUuid );
                    if ( li != NULL )
                        item->group( li );
                    linkedItem = linkedItem.nextSiblingElement();
                }
            }
        }
        else
            qWarning() << "No such timeline item:" << uuid;
        elem = elem.nextSiblingElement();
    }
}

void
Timeline::dragEnterEvent( QDragEnterEvent *event )
{
    if ( event->mimeData()->hasFormat("vlmc/effect_name") == true )
        event->acceptProposedAction();
    else
        event->ignore();
}

void
Timeline::dropEvent( QDropEvent *event )
{
    Effect* effect = EffectsEngine::getInstance()->effect( event->mimeData()->data( "vlmc/effect_name") );
    if ( effect != NULL )
        m_renderer->appendEffect( effect );
    else
        qWarning() << "Can't find effect name" << event->mimeData()->data( "vlmc/effect_name");
}
