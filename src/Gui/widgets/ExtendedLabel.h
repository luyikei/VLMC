/*****************************************************************************
 * ExtendedLabel.h: Provide a QLabel with elidable text in it.
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

#ifndef ELIDABLELABEL_H
#define ELIDABLELABEL_H

#include <QLabel>

class ExtendedLabel : public QLabel
{
    Q_OBJECT

    public:
        ExtendedLabel( QWidget* parent );
        ExtendedLabel( const QString& text, QWidget* parent );
        Qt::TextElideMode       elideMode() const;
        void                    setElideMode( Qt::TextElideMode mode );
        virtual QSize           minimumSizeHint() const;
        virtual QSize           sizeHint() const;
        void                    setText( const QString &text );

    protected:
        virtual void            resizeEvent( QResizeEvent *event );
        virtual void            mousePressEvent( QMouseEvent *ev );
        virtual void            mouseDoubleClickEvent( QMouseEvent *ev );

    private:
        Qt::TextElideMode       m_elideMode;
        QString                 m_text;

    signals:
        void                    clicked( QWidget* sender, QMouseEvent* ev );
        void                    doubleClicked();
};

#endif // ELIDABLELABEL_H
