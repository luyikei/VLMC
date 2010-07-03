/*****************************************************************************
 * ImportController.h
 *****************************************************************************
 * Copyright (C) 2008-2010 VideoLAN
 *
 * Authors: Geoffroy Lacarriere <geoffroylaca@gmail.com>
 *          Thomas Boquet <thomas.boquet@gmail.com>
 *          Hugo Beauzée-Luyssen <beauze.h@gmail.com>
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

/** \file
 * This file ImportController contains class declaration/definition.
 * It's the the controler of the import menu widget of vlmc.
 */

#ifndef IMPORTCONTROLLER_H
#define IMPORTCONTROLLER_H

#include "StackViewController.h"

#include <QDialog>
#include <QUuid>

class   Clip;
class   ClipMetadataDisplayer;
class   ClipRenderer;
class   Media;
class   MediaContainer;
class   MediaListView;
class   PreviewWidget;
class   TagWidget;

class   QFileSystemModel;
class   QModelIndex;

namespace Ui
{
    class ImportController;
}

/**
 *  \class ImportController
 *  \brief Controller of the import menu
 */
class ImportController : public QDialog
{
    Q_OBJECT

    public:
        ImportController(QWidget *parent = 0);
        ~ImportController();

    protected:
        void changeEvent( QEvent *e );

    private:
        void                        saveCurrentPath();
        void                        restoreCurrentPath();
        void                        collapseAllButCurrentPath();
        void                        importMedia( const QString &filePath );
        void                        importDir( const QString &path );
        void                        handleInvalidMedias();
    private:
        Ui::ImportController*       m_ui;
        StackViewController*        m_stackNav;
//        TagWidget*                  m_tag;
        MediaListView               *m_mediaListView;
        QFileSystemModel            *m_filesModel;
        QString                     m_currentlyWatchedDir;
        QUuid                       m_currentUuid;
        MediaContainer              *m_temporaryMedias;
        quint32                     m_nbMediaToLoad;
        quint32                     m_nbMediaLoaded;
        ClipRenderer*               m_clipRenderer;
        QStringList                 m_nameFilters;

    public slots:
        void        accept();
        void        reject();

    private slots:
        void        clipSelection( Clip* clip );
        void        forwardButtonClicked();
        void        treeViewClicked( const QModelIndex& index );
        void        treeViewDoubleClicked( const QModelIndex& index );
        void        mediaLoaded();
        void        failedToLoad( Media* media );
        void        hideErrors();

    signals:
        void        clipSelected( Clip* );
};

#endif // IMPORTCONTROLLER_H
