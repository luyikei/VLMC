/*****************************************************************************
 * WorkflowFileRenderer.h: Output the workflow to a file
 *****************************************************************************
 * Copyright (C) 2008-2016 VideoLAN
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

#ifndef WORKFLOWFILERENDERERDIALOG_H
#define WORKFLOWFILERENDERERDIALOG_H

#include <QDialog>
#include "ui/WorkflowFileRendererDialog.h"

class   RendererEventWatcher;

class   WorkflowFileRendererDialog : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY( WorkflowFileRendererDialog )
public:
    WorkflowFileRendererDialog( quint32 width, quint32 height );
    void    setOutputFileName( const QString& filename );
    void    setProgressBarValue( int val );

private:
    Ui::WorkflowFileRendererDialog      m_ui;
    quint32                             m_width;
    quint32                             m_height;

signals:
    void    stop();

public slots:
    void    updatePreview( const uchar* buff );
    void    frameChanged( qint64 newFrame, qint64 length );

private slots:
    void    cancel();
};

#endif // WORKFLOWFILERENDERERDIALOG_H
