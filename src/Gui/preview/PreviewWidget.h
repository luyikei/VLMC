/*****************************************************************************
 * PreviewWidget: Main preview widget
 *****************************************************************************
 * Copyright (C) 2008-2014 VideoLAN
 *
 * Authors: Hugo Beauzée-Luyssen <hugo@beauzee.fr>
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


#ifndef PREVIEWWIDGET_H
#define PREVIEWWIDGET_H

#include <QWidget>
#include "Workflow/MainWorkflow.h"

class GenericRenderer;
class RendererEventWatcher;

namespace Ui {
    class PreviewWidget;
}

class PreviewWidget : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY( PreviewWidget )

public:
    explicit PreviewWidget( QWidget* parent = NULL );
    virtual ~PreviewWidget();
    const GenericRenderer*  getGenericRenderer() const;
    void                    setRenderer( GenericRenderer *renderer );
    /**
     * @brief setClipEdition Allows to enable/disable markers & create clip buttons
     */
    void                    setClipEdition( bool enable );

private:
    Ui::PreviewWidget*      m_ui;
    GenericRenderer*        m_renderer;
    bool                    m_previewStopped;

protected:
    virtual void    changeEvent( QEvent *e );

public slots:
    void            stop();

private slots:
    void            on_pushButtonPlay_clicked();
    void            on_pushButtonStop_clicked();
    void            on_pushButtonNextFrame_clicked();
    void            on_pushButtonPreviousFrame_clicked();
    void            frameChanged( qint64, Vlmc::FrameChangedReason reason );
    void            videoPaused();
    void            videoPlaying();
    void            videoStopped();
    void            volumeChanged();
    void            updateVolume( int );
    void            markerStartClicked();
    void            markerStopClicked();
    void            createNewClipFromMarkers();
    void            error();
};

#endif // PREVIEWWIDGET_H
