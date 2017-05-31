/*****************************************************************************
 * WorkflowFileRendererDialog.cpp: Display a render feedback.
 *****************************************************************************
 * Copyright (C) 2008-2016 VideoLAN
 *
 * Authors: Hugo Beauzée-Luyssen <hugo@beauzee.fr>
 *          Christophe Courtaut <christophe.courtaut@gmail.com>
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "WorkflowFileRendererDialog.h"

#include "Main/Core.h"
#include "Project/Project.h"
#include "vlmc.h"
#include "Workflow/MainWorkflow.h"
#include "Tools/RendererEventWatcher.h"
#include "Backend/IInput.h"

WorkflowFileRendererDialog::WorkflowFileRendererDialog( quint32 width, quint32 height ) :
        m_width( width ),
        m_height( height )
{
    m_ui.setupUi( this );
    connect( m_ui.cancelButton, SIGNAL( clicked() ), this, SLOT( cancel() ) );
}

void
WorkflowFileRendererDialog::setOutputFileName( const QString& outputFileName )
{
    m_ui.nameLabel->setText( outputFileName );
    m_ui.previewLabel->setMinimumSize( m_width, m_height );
    setWindowTitle( "Rendering to " + outputFileName );
}

void
WorkflowFileRendererDialog::setProgressBarValue( int val )
{
    m_ui.progressBar->setValue( val );
}

void
WorkflowFileRendererDialog::updatePreview( const uchar* buff )
{
    QImage img( buff, m_width, m_height,
                QImage::Format_RGBA8888, []( void* buf ){ delete[] (uchar*) buf; } );
    m_ui.previewLabel->setPixmap( QPixmap::fromImage( img ) );
}

void
WorkflowFileRendererDialog::frameChanged( qint64 newFrame, qint64 length  )
{
    // Since frame is 0-indexed
    newFrame++;
    if ( newFrame <= length )
    {
        m_ui.frameCounter->setText( tr("Rendering frame %1 / %2").arg( QString::number( newFrame ),
                                        QString::number( length ) ) );
        setProgressBarValue( newFrame * 100 / length );
    }
}

void
WorkflowFileRendererDialog::cancel()
{
    emit stop();
    close();
}
