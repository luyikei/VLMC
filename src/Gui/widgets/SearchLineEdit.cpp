/*****************************************************************************
 * SearchLineEdit.cpp: A Line edit with a clear button
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

#include "SearchLineEdit.h"

#include "FramelessButton.h"
#include <QStyleOption>
#include <QPainter>

SearchLineEdit::SearchLineEdit( QWidget *parent ) : QLineEdit( parent )
{
    m_clearButton = new FramelessButton( this );
    m_clearButton->setIcon( QIcon( ":/images/clear" ) );
    m_clearButton->setIconSize( QSize( 16, 16 ) );
    m_clearButton->setCursor( Qt::ArrowCursor );
    m_clearButton->setToolTip( tr( "Clear" ) );
    m_clearButton->hide();

    connect( m_clearButton, SIGNAL( clicked() ), this, SLOT( clear() ) );

    int frameWidth = style()->pixelMetric( QStyle::PM_DefaultFrameWidth, 0, this );

    QFontMetrics metrics( font() );
    QString styleSheet = QString( "min-height: %1px; "
                                  "padding-top: 1px; "
                                  "padding-bottom: 1px; "
                                  "padding-right: %2px;" )
                                  .arg( metrics.height() + ( 2 * frameWidth ) )
                                  .arg( m_clearButton->sizeHint().width() + 1 );
    setStyleSheet( styleSheet );

    setMessageVisible( true );

    connect( this, SIGNAL( textEdited( const QString& ) ),
             this, SLOT( updateText( const QString& ) ) );
}

void
SearchLineEdit::clear()
{
    setText( QString() );
    m_clearButton->hide();
    setMessageVisible( true );
}

void
SearchLineEdit::setMessageVisible( bool on )
{
    m_message = on;
    repaint();
    return;
}

void
SearchLineEdit::updateText( const QString& text )
{
    m_clearButton->setVisible( !text.isEmpty() );
}

void
SearchLineEdit::resizeEvent ( QResizeEvent * event )
{
    QLineEdit::resizeEvent( event );

    int     frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth,0,this);
    m_clearButton->resize( m_clearButton->sizeHint().width(), height() );
    m_clearButton->move( width() - m_clearButton->width() - frameWidth, 0 );
}

void
SearchLineEdit::focusInEvent( QFocusEvent *event )
{
    if ( m_message )
    {
        setMessageVisible( false );
    }
    QLineEdit::focusInEvent( event );
}

void
SearchLineEdit::focusOutEvent( QFocusEvent *event )
{
    if ( text().isEmpty() == true )
    {
        setMessageVisible( true );
    }
    QLineEdit::focusOutEvent( event );
}

void
SearchLineEdit::paintEvent( QPaintEvent *event )
{
    QLineEdit::paintEvent( event );

    if( !m_message )
        return;
    QStyleOption    option;
    option.initFrom( this );
    QRect       rect = style()->subElementRect( QStyle::SE_LineEditContents, &option, this )
                    .adjusted( 3, 0, m_clearButton->width() + 1, 0 );

    QPainter    painter( this );
    painter.setPen( palette().color( QPalette::Disabled, QPalette::Text ) );
    painter.drawText( rect, Qt::AlignLeft | Qt::AlignVCenter, tr( "Filter" ) );
}
