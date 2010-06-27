/*****************************************************************************
 * ElidableLabel.cpp: Provide a QLabel with elidable text in it.
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

#include "ElidableLabel.h"

#include <QResizeEvent>

ElidableLabel::ElidableLabel( QWidget *parent ) : QLabel( parent ),
    m_elideMode( Qt::ElideMiddle )
{
}

ElidableLabel::ElidableLabel( const QString &text, QWidget *parent ) :
        QLabel( text, parent ),
        m_elideMode( Qt::ElideMiddle ),
        m_text( text )
{
    setToolTip( text );
}

void
ElidableLabel::resizeEvent( QResizeEvent *event )
{
    QFontMetrics fm( fontMetrics() );
    QString     str = fm.elidedText( m_text, m_elideMode, event->size().width() );
    QLabel::setText( str );
    QLabel::resizeEvent( event );
}

Qt::TextElideMode
ElidableLabel::elideMode() const
{
    return m_elideMode;
}

void
ElidableLabel::setElideMode( Qt::TextElideMode mode )
{
    m_elideMode = mode;
}

QSize
ElidableLabel::minimumSizeHint() const
{
    const QFontMetrics  &fm = fontMetrics();
    QSize               size( fm.width("..."), fm.height() );
    return size;
}

QSize
ElidableLabel::sizeHint() const
{
    const QFontMetrics& fm = fontMetrics();
    QSize size( fm.width( m_text ), fm.height());
    return size;
}

void
ElidableLabel::setText( const QString &text )
{
    m_text = text;
    setToolTip( text );
    QLabel::setText( text );
}
