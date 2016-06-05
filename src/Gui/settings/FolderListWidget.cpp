/*****************************************************************************
 * FolderListWidget.cpp
 *****************************************************************************
 * Copyright (C) 2008-2016 the VLMC team
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

#include "FolderListWidget.h"

#include <QHeaderView>
#include <QTreeView>
#include <QFileSystemModel>

#include "ui/FolderListWidget.h"

FolderList::FolderList( QWidget* parent )
    : QWidget( parent )
    , m_ui( new Ui::FolderListWidget )
{
    m_ui->setupUi( this );
    m_fsModel = new QFileSystemModel( this );
    m_fsModel->setRootPath( QDir::rootPath() );
    m_fsModel->setFilter( QDir::AllDirs | QDir::NoDotAndDotDot );
    m_ui->fsView->setModel( m_fsModel );

    m_ui->fsView->header()->setStretchLastSection( false );
    m_ui->fsView->header()->setSectionResizeMode( QHeaderView::ResizeToContents );
    m_ui->fsView->setColumnHidden( 1, true );
    m_ui->fsView->setColumnHidden( 2, true );
    m_ui->fsView->setColumnHidden( 3, true );

    connect( m_ui->addButton, &QPushButton::clicked, this, &FolderList::addFolder );
    connect( m_ui->removeButton, &QPushButton::clicked, this, &FolderList::removeFolder );
}

void
FolderList::addFolder()
{
    auto idx = m_ui->fsView->currentIndex();
    const auto path = m_fsModel->filePath( idx );
    if ( path.isEmpty() == true )
        return;
    auto items = m_ui->selectedFolders->findItems( path, Qt::MatchExactly );
    if ( items.size() > 0 )
        return;
    m_ui->selectedFolders->addItem( path );
    emit foldersChanged();
}

void
FolderList::removeFolder()
{
    auto idx = m_ui->selectedFolders->currentIndex();
    auto item = m_ui->selectedFolders->takeItem( idx.row() );
    if ( item == nullptr )
        return;
    emit foldersChanged();
    delete item;
}

QStringList
FolderList::folders()
{
    QStringList res;
    for ( auto i = 0; i < m_ui->selectedFolders->count(); ++i )
        res.append( m_ui->selectedFolders->item( i )->text() );
    return res;
}

void
FolderList::setFolders(QStringList folders)
{
    m_ui->selectedFolders->clear();
    for ( const auto& s : folders )
        m_ui->selectedFolders->addItem( s );
}

FolderListWidget::FolderListWidget( SettingValue* s, QWidget* parent )
    : ISettingsCategoryWidget( parent, s )
    , m_widget( new FolderList( this ) )
{
}

bool
FolderListWidget::save()
{
    m_setting->set( m_widget->property( "folders" ) );
    return true;
}

void
FolderListWidget::changed( const QVariant& value )
{
    m_widget->setProperty( "folders", value );
}
