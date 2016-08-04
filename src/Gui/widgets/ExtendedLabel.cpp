/*****************************************************************************
 * ExtendedLabel.cpp: Provide a QLabel with elidable text in it.
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

#include "ExtendedLabel.h"

#include <QResizeEvent>

ExtendedLabel::ExtendedLabel( QWidget *parent ) : QLabel( parent ),
    m_elideMode( Qt::ElideMiddle )
{
}

ExtendedLabel::ExtendedLabel( const QString &text, QWidget *parent ) :
        QLabel( text, parent ),
        m_elideMode( Qt::ElideMiddle ),
        m_text( text )
{
    setToolTip( text );
}

void
ExtendedLabel::resizeEvent( QResizeEvent *event )
{
    QFontMetrics fm( fontMetrics() );
    QString     str = fm.elidedText( m_text, m_elideMode, event->size().width() );
    QLabel::setText( str );
    QLabel::resizeEvent( event );
}

Qt::TextElideMode
ExtendedLabel::elideMode() const
{
    return m_elideMode;
}

void
ExtendedLabel::setElideMode( Qt::TextElideMode mode )
{
    m_elideMode = mode;
}

QSize
ExtendedLabel::minimumSizeHint() const
{
    if ( pixmap() != nullptr )
        return QLabel::sizeHint();
    const QFontMetrics  &fm = fontMetrics();
    QSize               size( fm.width("..."), fm.height() );
    return size;
}

QSize
ExtendedLabel::sizeHint() const
{
    if ( pixmap() != nullptr )
        return QLabel::sizeHint();
    const QFontMetrics& fm = fontMetrics();
    QSize size( fm.width( m_text ), fm.height());
    return size;
}

void
ExtendedLabel::setText( const QString &text )
{
    m_text = text;
    setToolTip( text );
    QLabel::setText( text );
}

void
ExtendedLabel::mousePressEvent( QMouseEvent* ev )
{
    emit clicked( this, ev );
}

void
ExtendedLabel::mouseDoubleClickEvent( QMouseEvent * )
{
    emit doubleClicked();
}
