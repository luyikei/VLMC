/*****************************************************************************
 * StackViewNavController.cpp
 *****************************************************************************
 * Copyright (C) 2008-2016 VideoLAN
 *
 * Authors: Thomas Boquet <thomas.boquet@gmail.com>
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

#include "StackViewNavController.h"
#include "ui/StackViewNavController.h"

StackViewNavController::StackViewNavController( QWidget *parent ) :
        QWidget( parent ),
    m_ui(new Ui::StackViewNavController)
{
    m_ui->setupUi(this);
    m_ui->previousButton->setHidden(true);
}

StackViewNavController::~StackViewNavController()
{
    delete m_ui;
}

void
StackViewNavController::changeEvent( QEvent *e )
{
    QWidget::changeEvent( e );
    switch ( e->type() )
    {
        case QEvent::LanguageChange:
            m_ui->retranslateUi( this );
            // TODO: Work on this quick and dirty fix.
            // But the title won't be translated anyway.
            setTitle( m_title );
            break;
        default:
            break;
    }
}

void
StackViewNavController::setTitle( const QString& title )
{
    m_title = title;
    m_ui->title->setText( title );
}

QPushButton*
StackViewNavController::previousButton() const
{
    return m_ui->previousButton;
}
