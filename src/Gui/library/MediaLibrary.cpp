/*****************************************************************************
 * MediaLibrary.cpp: VLMC media library's ui
 *****************************************************************************
 * Copyright (C) 2008-2010 VideoLAN
 *
 * Authors: Hugo Beauzée-Luyssen <beauze.h@gmail.com>
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

#include "MediaLibrary.h"

#include "Library.h"
#include "MediaListView.h"
#include "StackViewController.h"

#include <QtDebug>

MediaLibrary::MediaLibrary(QWidget *parent) : QWidget(parent),
    m_ui( new Ui::MediaLibrary() )
{
    m_ui->setupUi( this );
    StackViewController *nav = new StackViewController( m_ui->mediaListContainer );
    MediaListView       *mediaListView = new MediaListView( nav, Library::getInstance() );
    nav->pushViewController( mediaListView );

    connect( m_ui->importButton, SIGNAL( clicked() ),
             this, SIGNAL( importRequired() ) );
    connect( mediaListView, SIGNAL( clipSelected( Clip* ) ),
             this, SIGNAL( clipSelected( Clip* ) ) );
    connect( m_ui->filterInput, SIGNAL( textChanged( const QString& ) ),
             this, SLOT( filterUpdated( const QString& ) ) );
    connect( m_ui->clearButton, SIGNAL( clicked() ),
             this, SLOT( clearFilter() ) );
}

void
MediaLibrary::filterUpdated( const QString &filter )
{
    qDebug() << "Filter updated:" << filter;
}

void
MediaLibrary::clearFilter()
{
    m_ui->filterInput->setText( "" );
}
