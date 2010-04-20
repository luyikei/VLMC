/*****************************************************************************
 * TracksScene.cpp: QGraphicsScene that contains the tracks
 *****************************************************************************
 * Copyright (C) 2008-2010 VideoLAN
 *
 * Authors: Ludovic Fauvet <etix@l0cal.com>
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

#include "TracksScene.h"

#include "Commands.h"
#include "GraphicsMovieItem.h"
#include "GraphicsAudioItem.h"
#include "SettingsManager.h"
#include "Timeline.h"
#include "UndoStack.h"

#include <QMessageBox>
#include <QKeyEvent>
#include <QGraphicsSceneContextMenuEvent>
#include <QPushButton>

TracksScene::TracksScene( QObject* parent ) : QGraphicsScene( parent )
{
}

void TracksScene::keyPressEvent( QKeyEvent* keyEvent )
{
    TracksView* tv = Timeline::getInstance()->tracksView();
    if ( !tv ) return;

    if ( keyEvent->modifiers() == Qt::NoModifier &&
         keyEvent->key() == Qt::Key_Delete &&
         selectedItems().size() >= 1 )
    {
        // Items deletion
        keyEvent->accept();
        askRemoveSelectedItems();
    }

    QGraphicsScene::keyPressEvent( keyEvent );
}

void TracksScene::contextMenuEvent( QGraphicsSceneContextMenuEvent* event )
{
    QGraphicsScene::contextMenuEvent( event );
    if ( event->isAccepted() )
        return; // Event handled by an item in the scene.

    //TODO Event not handled, create and show a menu here.
}

void
TracksScene::askRemoveSelectedItems()
{
    TracksView* tv = Timeline::getInstance()->tracksView();

    if ( !tv ) return;

    if ( VLMC_GET_BOOL( "general/ConfirmDeletion" ) == true )
    {
        QString message;
        if ( selectedItems().size() == 1 )
            message = tr("Confirm the deletion of the region?");
        else
            message = tr("Confirm the deletion of those regions?");

        QMessageBox msgBox;
        msgBox.setText( message );
        msgBox.addButton( tr( "Yes" ), QMessageBox::YesRole );
        QAbstractButton     *always = msgBox.addButton( tr( "Yes, don't ask me again" ),
                                                        QMessageBox::YesRole );
        QAbstractButton     *no = msgBox.addButton( tr( "No" ), QMessageBox::NoRole );
        msgBox.exec();
        QAbstractButton*    clicked = msgBox.clickedButton();

        if ( clicked == no )
            return ;
        if ( clicked == always )
        {
            SettingsManager::getInstance()->setImmediateValue( "general/ConfirmDeletion",
                                                                   false, SettingsManager::Vlmc );
        }
    }

    UndoStack::getInstance()->beginMacro( "Remove clip(s)" );

    QList<QGraphicsItem*> items = selectedItems();
    for (int i = 0; i < items.size(); ++i )
    {
        AbstractGraphicsMediaItem* item = qgraphicsitem_cast<AbstractGraphicsMediaItem*>( items.at(i) );
        if ( !item ) return;

        Commands::trigger( new Commands::MainWorkflow::RemoveClip( item->clipHelper(),
                                                                   item->trackNumber(),
                                                                   item->startPos(),
                                                                   item->mediaType() ) );
    }

    UndoStack::getInstance()->endMacro();
}
