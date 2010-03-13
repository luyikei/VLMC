/*****************************************************************************
 * ImportController.cpp
 *****************************************************************************
 * Copyright (C) 2008-2010 VideoLAN
 *
 * Authors: Geoffroy Lacarriere <geoffroylaca@gmail.com>
 *          Thomas Boquet <thomas.boquet@gmail.com>
 *          Hugo Beauzée-Luyssen <hugo@vlmc.org>
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

#include "ui_ImportController.h"

#include "ClipRenderer.h"
#include "ImportController.h"
#include "Library.h"
#include "MetaDataManager.h"
#include "MediaCellView.h"
#include "MediaListView.h"

#include <QPalette>
#include <QSettings>
#include <QTime>
#include <QTimer>

ImportController::ImportController(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::ImportController),
    m_nbMediaToLoad( 0 ),
    m_nbMediaLoaded( 0 )
{
    m_ui->setupUi(this);
    //The renderer will be deleted by the PreviewWidget
    m_clipRenderer = new ClipRenderer;
    m_preview = new PreviewWidget( m_clipRenderer, m_ui->previewContainer );
    m_stackNav = new StackViewController( m_ui->stackViewContainer );
    m_temporaryMedias = new MediaContainer;
    m_mediaListView = new MediaListView( m_stackNav, m_temporaryMedias );
    m_tag = new TagWidget( m_ui->tagContainer, 6 );
    m_filesModel = new QFileSystemModel( this );
    m_stackNav->pushViewController( m_mediaListView );

    QStringList filters;
    filters << Media::AudioExtensions.split(' ', QString::SkipEmptyParts)
            << Media::VideoExtensions.split(' ', QString::SkipEmptyParts)
            << Media::ImageExtensions.split(' ', QString::SkipEmptyParts);
    m_filesModel->setFilter( QDir::AllDirs | QDir::AllEntries | QDir::NoDotAndDotDot );
    m_filesModel->sort( 2, Qt::AscendingOrder );
    m_filesModel->sort( 0, Qt::AscendingOrder );
    m_filesModel->setNameFilters( filters );
    m_filesModel->setRootPath( "/" );
    m_filesModel->setNameFilterDisables( false );

    restoreCurrentPath();

    m_ui->treeView->setModel( m_filesModel );
    m_ui->treeView->setHeaderHidden( true );
    m_ui->treeView->setCurrentIndex( m_filesModel->index( m_currentlyWatchedDir ) );
    m_ui->treeView->setExpanded( m_ui->treeView->currentIndex() , true );
    m_ui->treeView->header()->setStretchLastSection( false );
    m_ui->treeView->header()->setResizeMode( QHeaderView::ResizeToContents );
    m_ui->treeView->setColumnHidden( 1, true );
    m_ui->treeView->setColumnHidden( 2, true );
    m_ui->treeView->setColumnHidden( 3, true );
    m_ui->forwardButton->setEnabled( true );

    m_ui->progressBar->setHidden( true );
    m_ui->errorLabelImg->hide();
    m_ui->errorLabel->hide();

    connect( m_ui->treeView, SIGNAL( clicked( QModelIndex ) ),
             this, SLOT( treeViewClicked( QModelIndex ) ) );
    connect( m_ui->treeView, SIGNAL( doubleClicked( QModelIndex ) ),
             this, SLOT( treeViewDoubleClicked( QModelIndex ) ) );
    connect( m_ui->forwardButton, SIGNAL( clicked() ),
             this, SLOT( forwardButtonClicked() ) );

    connect( this, SIGNAL( clipSelected( Clip* ) ),
             m_clipRenderer, SLOT( setClip( Clip* ) ) );
    connect( m_mediaListView, SIGNAL( clipSelected( Clip* ) ),
             this, SLOT( clipSelection( Clip* ) ) );
    connect( m_mediaListView, SIGNAL( clipDeleted( const QUuid& ) ),
             m_clipRenderer, SLOT( clipUnloaded( const QUuid& ) ) );

    connect( MetaDataManager::getInstance(), SIGNAL( failedToCompute( Media* ) ),
             this, SLOT( failedToLoad( Media* ) ) );
}

ImportController::~ImportController()
{
    delete m_preview;
    delete m_ui;
    delete m_stackNav;
    delete m_tag;
}

void
ImportController::changeEvent( QEvent *e )
{
    QDialog::changeEvent( e );
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
ImportController::clipSelection( Clip* clip )
{
    const QUuid& uuid = clip->uuid();
    if ( m_currentUuid == uuid )
        return ;
    Media*  media = clip->rootClip()->getMedia();
    setUIMetaData( clip->rootClip() );
    m_preview->stop();
    m_currentUuid = uuid;
    m_tag->clipSelected( clip );
    emit clipSelected( clip );
}

void
ImportController::setUIMetaData( const Clip* clip )
{
    if ( clip != NULL )
    {
        //Duration
        QTime   duration;
        duration = duration.addSecs( clip->lengthSecond() );
        m_ui->durationValueLabel->setText( duration.toString( "hh:mm:ss" ) );
        //Filename || title
        m_ui->nameValueLabel->setText( clip->getMedia()->fileInfo()->fileName() );
        m_ui->nameValueLabel->setWordWrap( true );
        setWindowTitle( clip->getMedia()->fileInfo()->fileName() + " " + tr( "properties" ) );
        //Resolution
        m_ui->resolutionValueLabel->setText( QString::number( clip->getMedia()->width() )
                                        + " x " + QString::number( clip->getMedia()->height() ) );
        //FPS
        m_ui->fpsValueLabel->setText( QString::number( clip->getMedia()->fps() ) );
    }
    else
    {
        m_ui->durationValueLabel->setText( "--:--:--" );
        m_ui->nameValueLabel->setText( "none" );
        m_ui->resolutionValueLabel->setText( "-- x --" );
        m_ui->fpsValueLabel->setText( "--" );
    }
}

void
ImportController::importMedia( const QString &filePath )
{
    if ( Library::getInstance()->mediaAlreadyLoaded( filePath ) == true )
        return ;
    ++m_nbMediaToLoad;
    m_ui->progressBar->setMaximum( m_nbMediaToLoad );

    Media* media = m_temporaryMedias->addMedia( filePath );
    m_temporaryMedias->addClip( media->baseClip() );
    if ( media )
        connect( media, SIGNAL( metaDataComputed( const Media* ) ),
                 this, SLOT( metaDataComputed( const Media* ) ) );
}

void
ImportController::importDir( const QString &path )
{
    QDir            dir( path );
    QFileInfoList   files = dir.entryInfoList( QDir::NoDotAndDotDot | QDir::Readable
                                               | QDir::AllEntries );

    foreach ( QFileInfo fInfo, files )
    {
        if ( fInfo.isDir() == true )
            importDir( fInfo.absolutePath() );
        else
        {
            QString ext = fInfo.suffix();

            if ( Media::AudioExtensions.contains( ext ) ||
                 Media::VideoExtensions.contains( ext ) ||
                 Media::ImageExtensions.contains( ext ) )
            {
                importMedia( fInfo.absoluteFilePath() );
            }
        }
    }
}

void
ImportController::forwardButtonClicked()
{
    QModelIndex     index = m_ui->treeView->selectionModel()->currentIndex();
    QString         filePath = m_filesModel->fileInfo( index ).filePath();

    if ( !m_filesModel->isDir( index ) )
        importMedia( filePath );
    else
        importDir( filePath );
}

void
ImportController::treeViewClicked( const QModelIndex& index )
{
    if ( m_filesModel->isDir( index ) )
    {
        m_currentlyWatchedDir = m_filesModel->filePath( index );
        saveCurrentPath();
    }
    m_ui->forwardButton->setEnabled( true );
}

void
ImportController::treeViewDoubleClicked( const QModelIndex& index )
{
    if ( m_filesModel->isDir( index ) == false )
        forwardButtonClicked();
}

void
ImportController::reject()
{
    m_preview->stop();
    m_mediaListView->clear();
    m_temporaryMedias->clear();
    collapseAllButCurrentPath();
    m_clipRenderer->setClip( NULL );
    done( Rejected );
}

void
ImportController::accept()
{
    m_mediaListView->clear();
    m_preview->stop();
    collapseAllButCurrentPath();
    foreach ( Clip* clip, m_temporaryMedias->clips().values() )
        Library::getInstance()->addClip( clip );
    m_temporaryMedias->removeAll();
    m_clipRenderer->setClip( NULL );
    done( Accepted );
}

void
ImportController::collapseAllButCurrentPath()
{
    m_ui->treeView->collapseAll();
    QStringList paths;
    for ( QDir directory( m_currentlyWatchedDir ); !directory.isRoot(); directory.cdUp() )
        paths.prepend( directory.absolutePath() );
    while ( paths.count() > 0 )
    {
        m_ui->treeView->setCurrentIndex( m_filesModel->index( paths.takeFirst() ) );
        m_ui->treeView->setExpanded( m_ui->treeView->currentIndex() , true );
    }
    m_ui->forwardButton->setEnabled( true );
}

void
ImportController::saveCurrentPath()
{
    QSettings s;
    s.setValue( "ImportPreviouslySelectPath", m_currentlyWatchedDir );
    s.sync();
}

void
ImportController::restoreCurrentPath()
{
    QSettings s;
    QVariant path = s.value( "ImportPreviouslySelectPath", QDir::homePath() );
    QFileInfo info = path.toString();
    m_currentlyWatchedDir = info.absoluteFilePath();
}

void
ImportController::mediaLoaded()
{
    ++m_nbMediaLoaded;
    if ( m_nbMediaLoaded == m_nbMediaToLoad )
    {
        m_nbMediaLoaded = 0;
        m_nbMediaToLoad = 0;
        m_ui->progressBar->hide();
    }
    else
    {
        if ( m_nbMediaToLoad > 3 )
            m_ui->progressBar->show();
        m_ui->progressBar->setValue( m_nbMediaLoaded );
    }

}

void
ImportController::failedToLoad( Media *media )
{
    m_ui->errorLabel->setText( tr( "Failed to load %1").arg(
            media->fileInfo()->fileName() ) );
    m_ui->errorLabelImg->show();
    m_ui->errorLabel->show();
    QTimer::singleShot( 3000, this, SLOT( hideErrors() ) );
    delete m_temporaryMedias->removeClip( media->baseClip() );
    delete media;
}

void
ImportController::hideErrors()
{
    m_ui->errorLabelImg->hide();
    m_ui->errorLabel->hide();
}

void
ImportController::metaDataComputed( const Media *media )
{
    setUIMetaData( media->baseClip() );
}
