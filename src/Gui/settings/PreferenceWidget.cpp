/*****************************************************************************
 * PreferenceWidget.cpp: Abstract class that will be used to save load / preferences
 * values.
 *****************************************************************************
 * Copyright (C) 2008-2016 VideoLAN
 *
 * Authors: Clement CHAVANCE <kinder@vlmc.org>
 *          Hugo Beauzée-Luyssen <hugo@beauzee.fr>
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

#include "ISettingsCategoryWidget.h"
#include "PreferenceWidget.h"

#include "BoolWidget.h"
#include "FolderListWidget.h"
#include "DoubleWidget.h"
#include "IntWidget.h"
#include "KeyboardShortcut.h"
#include "LanguageWidget.h"
#include "PathWidget.h"
#include "StringWidget.h"

#include <QFormLayout>
#include <QWidget>
#include <QLabel>
#include <QHash>
#include <QHashIterator>
#include <QEvent>

PreferenceWidget::PreferenceWidget( const QString &name, const char *label, Settings* settings,
                                    QWidget *parent ) :
    QScrollArea( parent ),
    m_category( label )
{
    QWidget     *container = new QWidget( this );
    Settings::SettingList    settingList = settings->group( name );
    QFormLayout *layout = new QFormLayout( container );
    layout->setFieldGrowthPolicy( QFormLayout::AllNonFixedFieldsGrow );

    foreach ( SettingValue* s, settingList )
    {
        //Do not display private variables
        if ( ( s->flags() & SettingValue::Private ) != 0 )
            continue ;
        ISettingsCategoryWidget    *widget = widgetFactory( s );
        QLabel                      *label = new QLabel( tr( s->name() ), this );
        label->setToolTip( tr( s->description() ) );
        m_labels.insert( s, label );
        widget->setToolTip( s->description() );
        layout->addRow( label, widget );
        m_settings.push_back( widget );
    }

    setWidget( container );
    setWidgetResizable( true );
    setFrameStyle( QFrame::NoFrame );
}

ISettingsCategoryWidget*
PreferenceWidget::widgetFactory( SettingValue *s )
{
    switch ( s->type() )
    {
    case SettingValue::KeyboardShortcut:
        return new KeyboardShortcut( s, this );
    case SettingValue::Language:
        return new LanguageWidget( s, this );
    case SettingValue::String:
        return new StringWidget( s, this );
    case SettingValue::Int:
        return new IntWidget( s, this );
    case SettingValue::Double:
        return new DoubleWidget( s, this );
    case SettingValue::Bool:
        return new BoolWidget( s, this );
    case SettingValue::Path:
        return new PathWidget( s, this );
    case SettingValue::List:
    {
        if ( s->flags().testFlag( SettingValue::Folders ))
            return new FolderListWidget( s, this );
        return nullptr;
    }
    default:
        return nullptr;
    }
}

bool
PreferenceWidget::save()
{
    foreach ( ISettingsCategoryWidget* w, m_settings )
    {
        if ( w->save() == false )
            return false;
    }
    return true;
}

void
PreferenceWidget::reset()
{
    foreach ( ISettingsCategoryWidget* w, m_settings )
        w->setting()->restoreDefault();
    discard();
    save();
}

void
PreferenceWidget::discard()
{
    foreach ( ISettingsCategoryWidget* w, m_settings )
        w->changed( w->setting()->get() );
}

const char*
PreferenceWidget::category() const
{
    return m_category;
}

void
PreferenceWidget::changeEvent( QEvent *e )
{
    switch ( e->type() )
    {
    case QEvent::LanguageChange:
        retranslateUi();
        break;
    default:
        break;
    }
}

void
PreferenceWidget::retranslateUi()
{
    QHashIterator<SettingValue*, QLabel*> i( m_labels );

    while ( i.hasNext() )
    {
        i.next();
        i.value()->setText( tr( i.key()->name() ) );
        i.value()->setToolTip( tr( i.key()->description() ) );
    }

    SettingsList::iterator      it = m_settings.begin();
    SettingsList::iterator      ite = m_settings.end();
    while ( it != ite )
    {
        (*it)->setToolTip( tr( (*it)->setting()->description() ) );
        ++it;
    }
}
