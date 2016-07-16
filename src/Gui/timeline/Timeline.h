/*****************************************************************************
 * Timeline.h: Widget that handle the tracks
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

#ifndef TIMELINE_H
#define TIMELINE_H

#include "Media/Clip.h"
#include "vlmc.h"
#include "ui_Timeline.h"
#include "Workflow/Types.h"

class MainWindow;
class QQuickView;

/**
 * \brief Entry point of the timeline widget.
 */
class Timeline : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY( Timeline )
public:
    explicit Timeline( MainWindow* parent = 0 );
    virtual ~Timeline();

    QWidget*            container();

public slots:
    /**
     * \brief Asks the workflow to clear itself.
     */
    void clear() {}
    /**
     * \brief Change the zoom level for all widgets of the timeline.
     * \param factor The zoom factor.
     */
    void changeZoom( int factor ) { Q_UNUSED( factor ) }
    /**
     * \brief Change the duration of the project.
     * \param duration Duration in frames.
     */
    void setDuration( int duration ) { Q_UNUSED( duration ) }
    /**
     * \brief Change the currently selected tool.
     */
    void setTool( ToolButtons button ) { Q_UNUSED( button ) }

protected:
    virtual void changeEvent( QEvent *e ) { Q_UNUSED( e ) }

private:
    static Timeline*    m_instance;
    QQuickView*         m_view;
    QWidget*            m_container;
};

#endif // TIMELINE_H
