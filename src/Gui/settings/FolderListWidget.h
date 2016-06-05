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

#ifndef FOLDERLISTWIDGET_H
#define FOLDERLISTWIDGET_H

#include "ISettingsCategoryWidget.h"

namespace Ui
{
    class FolderListWidget;
}

class QTreeView;
class QItemSelectionModel;
class QFileSystemModel;

class FolderList : public QWidget
{
    Q_OBJECT

public:
    FolderList( QWidget* parent = nullptr );
    Q_PROPERTY(QStringList folders READ folders WRITE setFolders USER true)

signals:
    void foldersChanged();

private:
    void addFolder();
    void removeFolder();
    QStringList folders();
    void setFolders(QStringList folders);


private:
    QFileSystemModel*       m_fsModel;

    Ui::FolderListWidget*   m_ui;
};

class FolderListWidget : public ISettingsCategoryWidget
{
    Q_OBJECT

public:
    FolderListWidget( SettingValue* s, QWidget* parent = nullptr );
    bool save() override;

private slots:
    virtual void        changed( const QVariant& ) override;

private:
    FolderList*         m_widget;
};

#endif // FOLDERLISTWIDGET_H
