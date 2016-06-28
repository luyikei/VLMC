/*****************************************************************************
 * MediaLibrary.cpp: VLMC media library's ui
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

#include "MediaLibraryView.h"

#include "Project/Project.h"
#include "Media/Clip.h"
#include "Library/Library.h"
#include "Main/Core.h"
#include "Media/Media.h"
#include "MediaCellView.h"
#include "ViewController.h"
#include "Tools/VlmcDebug.h"

#include <QListView>
#include <QUrl>
#include <QMimeData>

MediaLibraryView::MediaLibraryView(QWidget *parent) : QWidget(parent),
    m_ui( new Ui::MediaLibraryView() )
{
    m_ui->setupUi( this );
}

MediaLibraryView::~MediaLibraryView()
{
    delete m_ui;
}

void
MediaLibraryView::changeEvent( QEvent *e )
{
    QWidget::changeEvent( e );
    switch ( e->type() )
    {
        case QEvent::LanguageChange:
            m_ui->retranslateUi( this );
            break;
        default:
            break;
    }
}
