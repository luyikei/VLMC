/*****************************************************************************
 * StringWidget.h: Handle text settings.
 *****************************************************************************
 * Copyright (C) 2008-2010 VideoLAN
 *
 * Authors: Hugo Beauzée-Luyssen <hugo@vlmc.org>
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

#include "PathWidget.h"
#include "SettingValue.h"

#include <QFileDialog>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>

PathWidget::PathWidget( SettingValue *s, QWidget *parent /*= NULL*/ ) :
        ISettingsCategoryWidget( s )
{
    m_widget = new QWidget( parent );
    QHBoxLayout *layout = new QHBoxLayout;
    m_lineEdit = new QLineEdit( m_widget );
    m_pushButton = new QPushButton( m_widget );
    m_pushButton->setText( tr( "Select a path" ) );
    layout->addWidget( m_lineEdit );
    layout->addWidget( m_pushButton );
    m_widget->setLayout( layout );

    changed( s->get() );
    connect( m_pushButton, SIGNAL( clicked() ), this, SLOT( selectPathButtonPressed() ) );
}

QWidget*
PathWidget::widget()
{
    return m_widget;
}

void
PathWidget::save()
{
    m_setting->set( m_lineEdit->text() );
}
void
PathWidget::changed( const QVariant &val )
{
    m_lineEdit->setText( val.toString() );
}

void
PathWidget::selectPathButtonPressed()
{
    QString path = QFileDialog::getExistingDirectory( NULL, tr( "Select a path" ),
                                                      m_setting->get().toString() );
    m_lineEdit->setText( path );
}
