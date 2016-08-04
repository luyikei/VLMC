/*****************************************************************************
 * AbstractUndoStack.cpp: An abstract UndoStack implementation for CUI
 *****************************************************************************
 * Copyright (C) 2008-2016 VideoLAN
 *
 * Authors: Yikei Lu <luyikei.qmltu@gmail.com>
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

#include "AbstractUndoStack.h"
#include "Commands.h"

using namespace Commands;

AbstractUndoStack::AbstractUndoStack( QObject* parent )
    : QObject( parent )
    , m_index( -1 )
{

}

void
AbstractUndoStack::redo()
{
    if ( m_index >= m_stack.size() )
        return;
    m_stack[m_index]->redo();
    m_index++;
    _setClean( false );
}

void
AbstractUndoStack::undo()
{
    if ( m_index < 0 )
        return;
    m_stack[m_index]->undo();
    m_index--;
    _setClean( false );
}

void
AbstractUndoStack::push( Generic* command )
{
    if ( m_index == -1 )
        m_index = 0;
    while ( m_index < m_stack.size() )
        m_stack.pop();
    m_stack.push( command );
    command->redo();
    _setClean( false );
}

void
AbstractUndoStack::setClean()
{
    _setClean( true );
}

void
AbstractUndoStack::_setClean( bool val )
{
    if ( val != m_isClean )
        emit cleanChanged( val );
    m_isClean = val;
}
