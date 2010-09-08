/*****************************************************************************
 * TracksView.h: QGraphicsView that contains the TracksScene
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

#ifndef TRACKSVIEW_H
#define TRACKSVIEW_H

#include "vlmc.h"
#include "AbstractGraphicsMediaItem.h"
#include "GraphicsCursorItem.h"
#include "Types.h"

#include <QWidget>
#include <QGraphicsView>
#include <QSet>

class QWheelEvent;
class QGraphicsWidget;
class QGraphicsLinearLayout;

class   ClipHelper;
class   Effect;
class   EffectHelper;
class   GraphicsAudioItem;
class   GraphicsEffectItem;
class   GraphicsMovieItem;
class   MainWorkflow;
class   TracksScene;
class   TrackWorkflow;
class   WorkflowRenderer;

class   ItemPosition
{
public:
    ItemPosition() : m_track( -1 ), m_time( -1 )
    {

    }
    ItemPosition( qint32 track, qint64 time )
    {
        m_track = track;
        m_time = time;
    }

    qint32      track() const
    {
        return m_track;
    }
    qint64      time() const
    {
        return m_time;
    }

    void        setTrack( qint32 track )
    {
        m_track = track;
    }
    void        setTime( qint64 time )
    {
        m_time = time;
    }

    bool        isValid() const
    {
        if ( m_track < 0 || m_time < 0 )
            return false;
        return true;
    }

private:
    qint32      m_track;
    qint64      m_time;
};

/**
 * \brief Class managing the timeline using QGraphicsItems.
 */
class TracksView : public QGraphicsView
{
    Q_OBJECT

public:
    enum    Action
    {
        None,
        Move,
        Resize,
    };

    TracksView( QGraphicsScene *scene, MainWorkflow *mainWorkflow, WorkflowRenderer *renderer, QWidget *parent = 0 );
    /**
     * \brief Set the duration of the project.
     * \param duration Duration in frames.
     * \sa duration
     */
    void setDuration( int duration );
    /**
     * \brief Get the duration of the project.
     * \return The duration in frames.
     * \sa setDuration
     */
    int duration() const { return m_projectDuration; }
    /**
     * \brief Get the current tracks' height.
     * \return The size in pixels.
     */
    int tracksHeight() const { return m_tracksHeight; }
    /**
     * \brief Change the position of the cursor.
     * \param pos The position where to move the cursor, in frames.
     * \sa cursorPos
     */
    void setCursorPos( qint64 pos );
    /**
     * \brief Get the current position of the cursor.
     * \return The current frame;
     * \sa setCursorPos
     */
    qint64 cursorPos();
    /**
     * \brief Return a pointer to the cursor.
     * \return A pointer to the GraphicsCursorItem.
     */
    GraphicsCursorItem *tracksCursor() const { return m_cursorLine; }
    /**
     * \brief Change the scale factor of the timeline.
     * \sa Timeline::changeZoom
     */
    void setScale( double scaleFactor );
    /**
     * \brief Return the list of all the AbstractGraphicsMediaItem contained
     *        in the timeline at the given position.
     * \param pos The position to look at.
     * \return A list of pointer to AbstractGraphicsMediaItem.
     * \warning Calling this method can be excessively slow!
     * \sa mediaItems()
     */
    template <typename T>
    QList<T*>       mediaItems( const QPoint &pos )
    {
        //TODO optimization needed!
        QList<QGraphicsItem*>               collisionList = items( pos );
        QList<T*>                           mediaCollisionList;

        for ( int i = 0; i < collisionList.size(); ++i )
        {
            T   *item = dynamic_cast<T*>( collisionList.at( i ) );
            if ( item != NULL )
                mediaCollisionList.append( item );
        }
        return mediaCollisionList;
    }

    /**
     * \brief This is an overloaded method provided for convenience.
     * \warning Calling this method can be excessively slow!
     * \sa mediaItems( const QPoint& pos )
     */
    QList<AbstractGraphicsItem*> timelineItems();
    /**
     * \brief Change the currently selected tool.
     * \param button The selected tool button.
     * \sa tool, ToolButtons
     */
    void                    setTool( ToolButtons button );
    /**
     * \brief Return the currently selected tool.
     * \return Selected tool button.
     * \sa setTool, ToolButtons
     */
    ToolButtons             tool() { return m_tool; }
    /**
     * \brief Get the WorkflowRenderer used by the timeline.
     * \return A pointer to the current WorkflowRenderer.
     */
    WorkflowRenderer        *getRenderer() { return m_renderer; }
    /**
     * \brief Remove a Clip from the timeline (and from the backend).
     * \param uuid The unique identifier of the Media.
     */
    void                    removeClip( const QUuid& uuid );

    /**
     *  \returns            The AbstractGraphicsMediaItem identified by the given uuid.
     *                      or NULL if there's no such item.
     *  \param              uuid    The ClipHelper's uuid
     */
    AbstractGraphicsMediaItem*  item( const QUuid& uuid );

public slots:
    /**
     * \brief Remove all items from the timeline.
     * \sa Timeline::clear
     */
    void                    clear();
    /**
     * \brief Insert an item into the timeline.
     * \param tw        The track the item was added in.
     * \param helper    The helper's item.
     * \param start     The position in frames.
     */
    void                    addItem( TrackWorkflow* tw, Workflow::Helper *helper, qint64 start );
    /**
     * \brief Move an item in the timeline.
     * \param tw    The TrackWorkflow in which the track was moved
     * \param ch    The clip that was moved.
     * \param time  The new position (in frames) of the item.
     */
    void                    moveItem( TrackWorkflow *tw, const QUuid& uuid, qint64 time );
    /**
     * \brief Remove an item from the timeline.
     * \param tw    The targeted TrackWorkflow
     * \param uuid The Universally Unique Identifier of the item.
     */
    void                    removeItem( TrackWorkflow* tw, const QUuid& uuid );
    /**
     * \brief This is an overloaded method provided for convenience.
     * \param item A pointer to AbstractGraphicsMediaItem.
     * \sa removeMediaItem( const QList<AbstractGraphicsMediaItem*>& )
     */
    void                    removeItem( AbstractGraphicsItem *item );

protected:
    virtual void            resizeEvent( QResizeEvent *event );
    virtual void            drawBackground( QPainter *painter, const QRectF &rect );
    virtual void            mouseMoveEvent( QMouseEvent *event );
    virtual void            mousePressEvent( QMouseEvent *event );
    virtual void            mouseReleaseEvent( QMouseEvent *event );
    virtual void            wheelEvent( QWheelEvent *event );
    virtual void            dragEnterEvent( QDragEnterEvent *event );
    virtual void            dragMoveEvent( QDragMoveEvent *event );
    virtual void            dragLeaveEvent( QDragLeaveEvent *event );
    virtual void            dropEvent( QDropEvent *event );

    void                    clipDragEnterEvent( QDragEnterEvent *event );
    void                    effectDragEnterEvent( QDragEnterEvent *event );

private slots:
    /**
     * \brief Ensure that the cursor is visible.
     */
    void                    ensureCursorVisible();

    /**
     * \brief Return the visible area of the viewport.
     */
    QRectF                  visibleRect();
    /**
     * \brief Update the global duration of the project.
     * This method should be called when an item is inserted, moved or removed to ensure
     * that the global time calculation is up-to-date.
     */
    void                    updateDuration();

    /**
     * \brief PLEASE DOCUMENT ME
     */
    void                    cleanUnusedTracks();
    /**
     * \brief Split an item in two at the given position.
     * \details Internally, the item given as parameter will be shrinked and a new one will be
     * appended at the end.
     * \param item The item.
     * \param frame the frame number where the cut should takes place.
     */
    void                    split( AbstractGraphicsMediaItem *item, qint64 frame );

private:
    /**
     * \brief Create the initial layout of the tracks
     */
    void                    createLayout();
    /**
     * \brief Insert an empty video track.
     */
    void                    addTrack( Workflow::TrackType type );
    /**
     * \brief DOCUMENT ME
     */
    void                    removeVideoTrack();
    /**
     * \brief DOCUMENT ME
     */
    void                    removeAudioTrack();
    /**
     * \brief DOCUMENT ME
     */
    void                    cleanTracks( Workflow::TrackType type );

    /**
     * \brief This is an overloaded method provided for convenience.
     * \param item Item to move.
     * \param position New position of the item.
     * \sa moveMediaItem( const QUuid& uuid, unsigned int track, qint64 time );
     */
    void                    moveItem( AbstractGraphicsItem *item, QPoint position );
    /**
     * \brief This is an overloaded method provided for convenience.
     * \param item Item to move.
     * \param track The new track of the item.
     * \param time The new position (in frames) of the item.
     * \sa moveMediaItem( const QUuid& uuid, unsigned int track, qint64 time );
     */
    void                    moveItem( AbstractGraphicsItem *item, qint32 track, qint64 time );

    ItemPosition            findPosition( AbstractGraphicsItem *item, qint32 track, qint64 time );

    /**
     * \brief Return a pointer to the specified track.
     * \param type The track's type.
     * \param number The track number.
     * \return A pointer to the GraphicsTrack.
     */
    GraphicsTrack           *getTrack( Workflow::TrackType type, unsigned int number );
    QGraphicsScene          *m_scene;
    int                     m_tracksHeight;
    int                     m_projectDuration;
    GraphicsCursorItem      *m_cursorLine;
    QGraphicsLinearLayout   *m_layout;
    qint32                  m_numVideoTrack;
    qint32                  m_numAudioTrack;
    MainWorkflow            *m_mainWorkflow;
    GraphicsMovieItem       *m_dragVideoItem;
    GraphicsAudioItem       *m_dragAudioItem;
    GraphicsEffectItem      *m_dragEffectItem;

    QGraphicsWidget         *m_separator;
    ToolButtons             m_tool;
    WorkflowRenderer        *m_renderer;

    // Mouse actions on Medias
    Action                  m_action;
    int                     m_actionRelativeX;
    AbstractGraphicsItem::From  m_actionResizeType;
    AbstractGraphicsItem    *m_actionItem;
    GraphicsTrack           *m_lastKnownTrack;
    QSet<QUuid>             m_itemsLoaded;

signals:
    /**
     * \brief Emitted when the zoom level has changed.
     */
    void                    zoomIn();
    /**
     * \brief Emitted when the zoom level has changed.
     */
    void                    zoomOut();
    /**
     * \brief Emitted when a new duration has been computed.
     */
    void                    durationChanged( int duration );
    /**
     * \brief Emitted when a video track has been added.
     * \param track A pointer to the newly added track.
     */
    void                    videoTrackAdded( GraphicsTrack *track );
    /**
     * \brief Emitted when an audio track has been added.
     * \param track A pointer to the newly added track.
     */
    void                    audioTrackAdded( GraphicsTrack *track );

    /**
     * \brief DOCUMENT ME
     */
    void                    videoTrackRemoved();
    /**
     * \brief DOCUMENT ME
     */
    void                    audioTrackRemoved();

friend class Timeline;
friend class TracksScene;
friend class AbstractGraphicsMediaItem;
};

#endif // TRACKSVIEW_H
