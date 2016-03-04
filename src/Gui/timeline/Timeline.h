/*****************************************************************************
 * Timeline.h: Widget that handle the tracks
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

#ifndef TIMELINE_H
#define TIMELINE_H

#include "Workflow/ClipHelper.h"
#include "Project/ILoadSave.h"
#include "vlmc.h"
#include "ui_Timeline.h"
#include "Workflow/Types.h"

#include <QDomElement>
#include <QXmlStreamWriter>

class   MainWorkflow;
class   Project;
class   TracksScene;
class   TracksView;
class   TracksControls;
class   TracksRuler;
class   WorkflowRenderer;

/**
 * \brief Entry point of the timeline widget.
 */
class Timeline : public QWidget, public ILoadSave
{
    Q_OBJECT
    Q_DISABLE_COPY( Timeline )
public:
    explicit Timeline( QWidget *parent = 0 );
    virtual ~Timeline();
    /// Return a pointer to the TracksView instance.
    TracksView*         tracksView() { return m_tracksView; }
    /// Returns a const pointer to the TracksView instance
    const TracksView*   tracksView() const { return m_tracksView; }
    /// Return a pointer to the TracksScene instance.
    TracksScene*        tracksScene() { return m_tracksScene; }
    /// Return a pointer to the TracksRuler instance.
    TracksRuler*        tracksRuler() { return m_tracksRuler; }
    /// Return a pointer to the Timeline instance (singleton).
    static Timeline*    getInstance() { return m_instance; }
    WorkflowRenderer    *renderer() { return m_renderer; }

public slots:
    /**
     * \brief Asks the workflow to clear itself.
     */
    void clear();
    /**
     * \brief Change the zoom level for all widgets of the timeline.
     * \param factor The zoom factor.
     */
    void changeZoom( int factor );
    /**
     * \brief Change the duration of the project.
     * \param duration Duration in frames.
     */
    void setDuration( int duration );
    /**
     * \brief Change the currently selected tool.
     */
    void setTool( ToolButtons button );

protected:
    virtual void changeEvent( QEvent *e );

private:
    void                initialize();
    virtual bool        save( QXmlStreamWriter& project );
    virtual bool        load( const QDomDocument& root );


private:
    Ui::Timeline        m_ui;
    TracksView*         m_tracksView;
    TracksScene*        m_tracksScene;
    TracksRuler*        m_tracksRuler;
    TracksControls*     m_tracksControls;
    double              m_scale;
    MainWorkflow*       m_mainWorkflow;
    WorkflowRenderer*   m_renderer;
    static Timeline*    m_instance;
};

#endif // TIMELINE_H
