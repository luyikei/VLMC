/*****************************************************************************
 * ClipProperty.cpp: Handle the clip properties and meta tags edition
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

#include "ClipProperty.h"
#include "ui/ClipProperty.h"

#include "Media/Clip.h"
#include "Media/Media.h"

#include "Gui/media/ClipMetadataDisplayer.h"

#include <QInputDialog>
#include <QPushButton>
#include <QRegExp>

ClipProperty::ClipProperty( Clip* clip, QWidget *parent ) :
    QDialog( parent ),
    ui( new Ui::ClipProperty ),
    m_clip( clip )
{
    ui->setupUi(this);
    connect( this, SIGNAL( accepted() ), this, SLOT( deleteLater() ) );
    connect( this, SIGNAL( rejected() ), this, SLOT( deleteLater() ) );

    setWindowTitle( m_clip->media()->fileInfo()->fileName() + " " + tr( "properties" ) );
    //Snapshot
    ui->snapshotLabel->setPixmap( m_clip->media()->snapshot().scaled( 128, 128, Qt::KeepAspectRatio ) );
    //Metatags
    const QPushButton* button = ui->buttonBox->button( QDialogButtonBox::Apply );
    Q_ASSERT( button != nullptr);
    connect( button, SIGNAL( clicked() ), this, SLOT( apply() ) );

    m_model = new QStringListModel( m_clip->metaTags(), this );
    ui->metaTagsView->setModel( m_model );

    //Notes:
    ui->annotationInput->setPlainText( m_clip->notes() );

    connect( ui->addTagsButton, SIGNAL( clicked() ), this, SLOT( addTagsRequired() ) );
    connect( ui->deleteTagsButton, SIGNAL( clicked() ), this, SLOT( removeTagsRequired() ) );

    ui->metadataContainer->setWatchedClip( clip );
}

ClipProperty::~ClipProperty()
{
    delete ui;
}

void
ClipProperty::changeEvent( QEvent *e )
{
    QDialog::changeEvent( e );
    switch ( e->type() )
    {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void
ClipProperty::apply()
{
    m_clip->setNotes( ui->annotationInput->toPlainText() );
    m_clip->setMetaTags( m_model->stringList() );
}

void
ClipProperty::addTagsRequired()
{
    bool                ok;
    QString             newTags = QInputDialog::getText( this, tr( "New tags edition" ),
                                            tr( "Enter tags (you can enter multiple tags, separated by a comma)" ),
                                            QLineEdit::Normal, "", &ok );
    if ( ok == true && newTags.length() > 0 )
    {
        QStringList         list = m_model->stringList();
        QRegExp             regexp( "\\s*,\\s*" );
        QStringList         toAdd = newTags.split( regexp, QString::SkipEmptyParts );
        list.append( toAdd );
        m_model->setStringList( list );
    }
}

void
ClipProperty::removeTagsRequired()
{
    QItemSelectionModel     *selected = ui->metaTagsView->selectionModel();
    QModelIndexList         listSelected = selected->selectedIndexes();
    QStringList             list = m_model->stringList();
    while ( listSelected.empty() == false )
    {
        QVariant    elem = m_model->data( listSelected.first(), Qt::DisplayRole );
        list.removeOne( elem.toString() );
        listSelected.removeFirst();
    }
    m_model->setStringList( list );
}
