/*****************************************************************************
 * SettingsDialog.h: generic preferences interface
 *****************************************************************************
 * Copyright (C) 2008-2016 VideoLAN
 *
 * Authors: Clement CHAVANCE <kinder@vlmc.org>
 *          Ludovic Fauvet <etix@l0cal.com>
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

#ifndef SETTINGS_H
#define SETTINGS_H

#include "Settings/Settings.h"

#include <QDialog>
#include <QString>

class QAbstractButton;
class QDialogButtonBox;
class QGridLayout;
class QLabel;
class QScrollArea;
class QStackedLayout;

class   Panel;
class   PreferenceWidget;

class   SettingsDialog : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY( SettingsDialog );

    public:
        SettingsDialog(Settings *settings, const QString& title, QWidget *parent = 0 );

        void                        addCategory( const QString& categorieName,
                                                 const char *label,
                                                 const QIcon &icon );

    protected:
        void                        changeEvent( QEvent *e );
    private:
        void                        buildLayout();
        void                        retranslateUi();

    private:
        QDialogButtonBox            *m_buttons;
        Panel                       *m_panel;
        QLabel                      *m_title;
        QStackedLayout              *m_stackedLayout;
        Settings*                   m_settings;
        QString                     m_windowTitle;

    public slots:
        void    switchWidget( int index );

    private slots:
        void    buttonClicked( QAbstractButton *button );

    signals:
        void    loadSettings();
};

#endif // SETTINGS_H
