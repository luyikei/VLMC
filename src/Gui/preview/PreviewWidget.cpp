/*****************************************************************************
 * PreviewWidget.cpp : Main widget for preview. Will dispatch on Clip or
 *                     Render preview
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <memory>

#include "Media/Clip.h"
#include "Media/Media.h"
#include "Renderer/ClipRenderer.h"
#include "Backend/MLT/MLTOutput.h"
#include "PreviewWidget.h"
#include "PreviewRuler.h"
#include "RenderWidget.h"
#include "Tools/RendererEventWatcher.h"
#include "ui/PreviewWidget.h"

#include <QMessageBox>
#include <QLayout>

PreviewWidget::PreviewWidget( QWidget *parent )
    : QWidget( parent )
    , m_ui( new Ui::PreviewWidget )
    , m_renderer( nullptr )
    , m_previewStopped( true )
{
    m_ui->setupUi( this );

    // Prevent buttons overlap on MacOS
    for (int i = 0; i < m_ui->toolsWidget->layout()->count(); ++i)
    {
        QWidget* w = m_ui->toolsWidget->layout()->itemAt( i )->widget();
        if ( w != nullptr )
            w->setAttribute( Qt::WA_LayoutUsesWidgetRect );
    }

    connect( m_ui->rulerWidget, SIGNAL( timeChanged(int,int,int,int) ),
             m_ui->lcdNumber,   SLOT( setTime(int,int,int,int) ) );

    connect( m_ui->pushButtonMarkerStart, SIGNAL( clicked() ), this, SLOT( markerStartClicked() ) );
    connect( m_ui->pushButtonMarkerStop, SIGNAL( clicked() ), this, SLOT( markerStopClicked() ) );
    connect( m_ui->pushButtonCreateClip, SIGNAL( clicked() ), this, SLOT( createNewClipFromMarkers() ) );
}

PreviewWidget::~PreviewWidget()
{
    delete m_ui;
}

void
PreviewWidget::setRenderer( AbstractRenderer* renderer )
{
    delete m_renderer;
    m_renderer = renderer;

    // Give the renderer to the ruler
    m_ui->rulerWidget->setRenderer( m_renderer );
    auto output = new Backend::MLT::MLTSdlOutput;
    output->setWindowId( (intptr_t)m_ui->renderWidget->id() );
    m_renderer->setOutput( std::unique_ptr<Backend::IOutput>( output ) );

#if defined ( Q_OS_MAC )
    /* Releases the NSView in the RenderWidget*/
    m_ui->renderWidget->release();
#endif

    connect( renderer->eventWatcher(), SIGNAL( stopped() ), this, SLOT( videoStopped() ) );
    connect( renderer->eventWatcher(), SIGNAL( paused() ), this, SLOT( videoPaused() ) );
    connect( renderer->eventWatcher(), SIGNAL( playing() ), this, SLOT( videoPlaying() ) );
    connect( renderer->eventWatcher(), SIGNAL( errorEncountered() ), this, SLOT( error() ) );
    connect( renderer->eventWatcher(), SIGNAL( volumeChanged() ), this, SLOT( volumeChanged() ) );

    connect( m_ui->rulerWidget, SIGNAL( frameChanged(qint64, Vlmc::FrameChangedReason) ),
             m_renderer,       SLOT( previewWidgetCursorChanged(qint64) ) );

    connect( m_ui->volumeSlider, SIGNAL( valueChanged ( int ) ),
             this, SLOT( updateVolume( int ) ) );
    connect( m_ui->volumeSlider, SIGNAL( sliderMoved( int ) ),
             this, SLOT( updateVolume( int ) ) );
}

void
PreviewWidget::setClipEdition( bool enable )
{
    m_ui->pushButtonMarkerStart->setVisible( enable );
    m_ui->pushButtonMarkerStop->setVisible( enable );
    m_ui->pushButtonCreateClip->setVisible( enable );
}

void
PreviewWidget::changeEvent( QEvent *e )
{
    switch ( e->type() )
    {
    case QEvent::LanguageChange:
        m_ui->retranslateUi( this );
        break;
    default:
        break;
    }
}

void
PreviewWidget::frameChanged( qint64 currentFrame, Vlmc::FrameChangedReason reason )
{
    if ( reason != Vlmc::PreviewCursor )
    {
        m_ui->rulerWidget->setFrame( currentFrame, false );
    }
}

void
PreviewWidget::on_pushButtonPlay_clicked()
{
    if ( m_previewStopped == true )
        m_previewStopped = false;
    m_renderer->togglePlayPause();
}

void
PreviewWidget::videoPaused()
{
    m_ui->pushButtonPlay->setIcon( QIcon( ":/images/play" ) );
}

void
PreviewWidget::videoStopped()
{
    m_ui->pushButtonPlay->setIcon( QIcon( ":/images/play" ) );
}

void
PreviewWidget::videoPlaying()
{
    m_ui->pushButtonPlay->setIcon( QIcon( ":/images/pause" ) );
}

void
PreviewWidget::volumeChanged()
{
    int volume = m_renderer->getVolume();
    m_ui->volumeSlider->setValue( volume );
}

void
PreviewWidget::updateVolume( int volume )
{
    // Returns 0 if the volume was set, -1 if it was out of range
    m_renderer->setVolume( volume );
}

void
PreviewWidget::on_pushButtonNextFrame_clicked()
{
    if ( m_previewStopped == false )
        m_renderer->nextFrame();
}

void
PreviewWidget::on_pushButtonPreviousFrame_clicked()
{
    if ( m_previewStopped == false )
        m_renderer->previousFrame();
}

const AbstractRenderer*
PreviewWidget::getAbstractRenderer() const
{
    return m_renderer;
}

void
PreviewWidget::stop()
{
    if ( m_previewStopped == false )
    {
        m_previewStopped = true;
        m_renderer->stop();
    }
}

void
PreviewWidget::on_pushButtonStop_clicked()
{
    stop();
}

void
PreviewWidget::markerStartClicked()
{
    m_ui->rulerWidget->setMarker( PreviewRuler::Start );

    qint64  beg = m_ui->rulerWidget->getMarker( PreviewRuler::Start );
    qint64  end = m_ui->rulerWidget->getMarker( PreviewRuler::Stop );
    if ( beg > end )
    {
        m_ui->rulerWidget->hideMarker( PreviewRuler::Stop );;
    }
}

void
PreviewWidget::markerStopClicked()
{
    m_ui->rulerWidget->setMarker( PreviewRuler::Stop );
    qint64  beg = m_ui->rulerWidget->getMarker( PreviewRuler::Start );
    qint64  end = m_ui->rulerWidget->getMarker( PreviewRuler::Stop );
    if ( beg > end )
    {
        m_ui->rulerWidget->hideMarker( PreviewRuler::Start );;
    }
}

void
PreviewWidget::createNewClipFromMarkers()
{
    ClipRenderer* clipRenderer = qobject_cast<ClipRenderer*>( m_renderer );
    Q_ASSERT( clipRenderer != nullptr );

    Clip* clip = clipRenderer->getClip();
    if ( clip == nullptr )
        return ;
    auto media = clip->media();
    qint64  beg = m_ui->rulerWidget->getMarker( PreviewRuler::Start );
    qint64  end = m_ui->rulerWidget->getMarker( PreviewRuler::Stop );

    if ( beg == -1 && end == -1 )
        return ;

    if ( end < beg )
        return ;

    beg = beg < 0 ? 0 : beg;
    Clip*   part = media->cut( beg, end );

    //Adding the newly created clip to the media
    if ( clip->addSubclip( part ) == false )
        delete part;
}

void
PreviewWidget::error()
{
    QMessageBox::warning( this, tr( "Rendering error" ),
                          tr( "An error occurred while rendering.\nPlease check your VLC installation"
                              " before reporting the issue.") );
}
