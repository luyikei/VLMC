/*****************************************************************************
 * AbstractUndoStack.h: An abstract UndoStack for both GUI and CUI
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

#ifndef ABSTRACTUNDOSTACK_H
#define ABSTRACTUNDOSTACK_H

#include <QObject>

#ifdef HAVE_GUI
#include <QUndoStack>
#include "Commands.h"
#else
#include <QStack>
#endif

namespace Commands
{
#ifdef HAVE_GUI
    class AbstractUndoStack : public QUndoStack
    {
#else
    class Generic;
    class AbstractUndoStack : public QObject
    {
        Q_OBJECT

        public:
            explicit AbstractUndoStack( QObject* parent = 0 );

        signals:
            void cleanChanged( bool val );

        public slots:
            void redo();
            void undo();
            void push( Generic* command );
            void setClean();

        private:
            // Avoid overloading setClean
            void _setClean( bool val );

            bool         m_isClean;
            QStack<Generic*>       m_stack;
            int          m_index;
#endif
    };
}

#endif // ABSTRACTUNDOSTACK_H
