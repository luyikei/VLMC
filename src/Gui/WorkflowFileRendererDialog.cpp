/*****************************************************************************
 * WorkflowFileRendererDialog.cpp: Display a render feedback.
 *****************************************************************************
 * Copyright (C) 2008-2014 VideoLAN
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

#include "WorkflowFileRendererDialog.h"

#include "vlmc.h"
#include "MainWorkflow.h"
#include "WorkflowFileRenderer.h"

WorkflowFileRendererDialog::WorkflowFileRendererDialog( WorkflowFileRenderer* renderer,
                                                        quint32 width, quint32 height ) :
        m_width( width ),
        m_height( height ),
        m_renderer( renderer )
{
    m_ui.setupUi( this );
    connect( m_ui.cancelButton, SIGNAL( clicked() ), this, SLOT( cancel() ) );
    connect( m_renderer, SIGNAL( renderComplete() ), this, SLOT( accept() ) );
    connect( m_renderer, SIGNAL( frameChanged( qint64 ) ), this, SLOT( frameChanged( qint64 ) ) );
    connect( m_renderer, SIGNAL( imageUpdated( const uchar* ) ),
             this, SLOT( updatePreview( const uchar* ) ),
             Qt::QueuedConnection );
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
    m_ui.previewLabel->setPixmap(
            QPixmap::fromImage( QImage( buff, m_width, m_height,
                                        QImage::Format_RGB32 ) ) );
}

void
WorkflowFileRendererDialog::frameChanged( qint64 frame )
{
    qint64 totalFrames = MainWorkflow::getInstance()->getLengthFrame();

    if ( frame <= totalFrames )
    {
        m_ui.frameCounter->setText( tr("Rendering frame %1 / %2").arg(QString::number( frame ),
                                        QString::number( totalFrames ) ) );
        setProgressBarValue( frame * 100 / totalFrames );
    }
}

void
WorkflowFileRendererDialog::cancel()
{
    m_renderer->stop();
    close();
}
