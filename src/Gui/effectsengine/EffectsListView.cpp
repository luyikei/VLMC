/*****************************************************************************
 * EffectsListView.cpp: Display a list of effects
 *****************************************************************************
 * Copyright (C) 2008-2010 VideoLAN
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

#include "EffectsListView.h"
#include "EffectsEngine.h"
#include "EffectWidget.h"

#include <QApplication>
#include <QDialog>
#include <QMouseEvent>
#include <QStandardItem>
#include <QVBoxLayout>

#include <QtDebug>

EffectsListView::EffectsListView( QWidget *parent ) :
    QListView(parent)
{
    m_model = new QStandardItemModel( this );
    connect( EffectsEngine::getInstance(),
             SIGNAL( effectAdded( Effect*, const QString&, Effect::Type ) ),
             this,
             SLOT( effectAdded(Effect*, const QString&, Effect::Type) ) );
    setModel( m_model );
    connect( this, SIGNAL( activated( QModelIndex ) ),
             this, SLOT( effectActivated( QModelIndex ) ) );
    setEditTriggers( QAbstractItemView::NoEditTriggers );
}

void
EffectsListView::effectAdded( Effect *, const QString& name, Effect::Type type )
{
    if ( type == m_type )
        m_model->appendRow( new QStandardItem( name ) );
}

void
EffectsListView::setType( Effect::Type type )
{
    m_type = type;
}

void
EffectsListView::mousePressEvent( QMouseEvent *event )
{
    QListView::mousePressEvent( event );

    if ( ( event->buttons() | Qt::LeftButton ) == Qt::LeftButton )
        m_dragStartPos = event->pos();
}

void
EffectsListView::mouseMoveEvent( QMouseEvent *event )
{
    if ( ( event->buttons() | Qt::LeftButton ) != Qt::LeftButton )
         return;

    if ( ( event->pos() - m_dragStartPos ).manhattanLength()
          < QApplication::startDragDistance() )
        return;

    QMimeData* mimeData = new QMimeData;
    mimeData->setData( "vlmc/effect_name", m_model->data( currentIndex() ).toByteArray() );
    QDrag* drag = new QDrag( this );
    drag->setMimeData( mimeData );
    drag->exec( Qt::CopyAction | Qt::MoveAction, Qt::CopyAction );
}

void
EffectsListView::effectActivated( const QModelIndex &index ) const
{
    if ( index.isValid() == false )
        return ;
    Effect  *effect = EffectsEngine::getInstance()->effect( m_model->data( index, Qt::DisplayRole ).toString() );
    QDialog         *dialog = new QDialog();
    QVBoxLayout     *layout = new QVBoxLayout( dialog );
    EffectWidget    *wid = new EffectWidget( dialog );
    layout->addWidget( wid );
    wid->setEffect( effect );
    dialog->setWindowTitle( tr( "%1 informations" ).arg( effect->name() ) );
    dialog->exec();
    delete dialog;
}
