/*****************************************************************************
 * SearchLineEdit.h: A Line edit with a clear button
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

#ifndef SEARCHLINEEDIT_H
#define SEARCHLINEEDIT_H

class   FramelessButton;

#include <QLineEdit>

class   QLabel;

class SearchLineEdit : public QLineEdit
{
    Q_OBJECT

    public:
        SearchLineEdit( QWidget *parent = nullptr );

    private:
        void    resizeEvent ( QResizeEvent * event );
        void    focusInEvent( QFocusEvent *event );
        void    focusOutEvent( QFocusEvent *event );
        void    paintEvent( QPaintEvent *event );
        void    setMessageVisible( bool on );

    private:
        FramelessButton     *m_clearButton;
        bool                m_message;

    public slots:
        void    clear();

    private slots:
        void    updateText( const QString& );
};

#endif // SEARCHLINEEDIT_H
