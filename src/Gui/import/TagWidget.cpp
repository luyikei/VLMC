/*****************************************************************************
 * TagWidget.cpp : Widget for tagging media
 *                     Render preview
 *****************************************************************************
 * Copyright (C) 2008-2016 VideoLAN
 *
 * Authors: Geoffroy Lacarriere <geoffroylaca@gmail.com>
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

#include "TagWidget.h"
#include "ui/TagWidget.h"

#include "Media/Media.h"
#include "Media/Clip.h"

#include <QPushButton>

TagWidget::TagWidget( QWidget *parent, int nbButton, QStringList tagList ) :
    QWidget( parent ),
    m_ui( new Ui::TagWidget ),
    m_nbButton( nbButton ),
    m_defaultTagList( tagList ),
    m_currentClip( nullptr )
{
    m_ui->setupUi( this );
    m_defaultTagList << "Outdoor" << "Holiday" << "Seaside" << "Sunset" << "Family";

    for( int i = 0; i < m_defaultTagList.count(); i++ )
    {
        if ( i < m_buttonList.count() )
        {
            static_cast<QPushButton*>( m_buttonList[i])->setText( m_defaultTagList[i] );
            static_cast<QPushButton*>( m_buttonList[i])->setEnabled( false );
        }
    }
    connect( m_ui->TagTextEdit, SIGNAL( textChanged() ), this, SLOT( setMetaTags() ) );
}

TagWidget::~TagWidget()
{
    delete m_ui;
}

void
TagWidget::clipSelected( Clip* clip )
{
    m_currentClip = clip;
    setTagTextEdit();
    for (int i = 0; i < m_buttonList.count(); i++)
    {
        static_cast<QPushButton*>( m_buttonList[i])->setEnabled( true );
        if ( m_currentClip->metaTags().contains( static_cast<QPushButton*>(m_buttonList[i])->text() ) )
            static_cast<QPushButton*>(m_buttonList[i])->setChecked( true );
        else
            static_cast<QPushButton*>(m_buttonList[i])->setChecked( false );
    }
    connect( clip->media(), SIGNAL( metaDataComputed( const Media* ) ),
             this, SLOT( setMetaTags() ) );
    connect( clip, SIGNAL( destroyed() ), this, SLOT( clipDestroyed() ) );
}

void
TagWidget::setMetaTags()
{
    if ( m_currentClip != nullptr )
    {
        QStringList tagList = m_ui->TagTextEdit->document()->toPlainText().split( ",", QString::SkipEmptyParts );
        m_currentClip->setMetaTags( tagList );
    }
}

void
TagWidget:: buttonTagClicked()
{
    if ( m_currentClip != nullptr )
    {
        QStringList tagList = m_currentClip->metaTags();
        for (int i = 0; i < m_buttonList.count(); i++)
        {
            QPushButton* button = static_cast<QPushButton*>(m_buttonList[i]);
            if ( button->isChecked() && !tagList.contains( button->text() ) )
                tagList << button->text();
            else if ( !button->isChecked() && tagList.contains( button->text() ) )
                tagList.removeAll( button->text() );
        }
        m_currentClip->setMetaTags( tagList );
        setTagTextEdit();
    }
}

void
TagWidget::setTagTextEdit()
{
    QString tags;
    if ( m_currentClip != nullptr )
    {
        for( int i = 0; i < m_currentClip->metaTags().count(); i++ )
        {
            if (i == 0)
                tags += m_currentClip->metaTags()[i];
            else
                tags += "," + m_currentClip->metaTags()[i];
        }
        m_ui->TagTextEdit->setText(tags);
        setButtonList( m_defaultTagList );
    }
}

void
TagWidget::setButtonList( QStringList tagList )
{
    if ( m_currentClip != nullptr )
    {
        for( int i = 0; i < tagList.count(); i++ )
        {
            if ( i < m_buttonList.count() )
                static_cast<QPushButton*>( m_buttonList[i] )->setText( tagList[i] );
        }
    }
}

void
TagWidget::changeEvent( QEvent *e )
{
    QWidget::changeEvent( e );
    switch ( e->type() )
    {
        case QEvent::LanguageChange:
            m_ui->retranslateUi( this );
            break;
        default:
            break;
    }
}

void
TagWidget::clipDestroyed()
{
    m_currentClip = nullptr;
}
