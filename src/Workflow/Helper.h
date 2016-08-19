/*****************************************************************************
 * Helper.h: Describes a common interface for all workflow helpers.
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

#ifndef IHELPER_H
#define IHELPER_H

#include <QObject>
#include <QUuid>
#include <QMetaType>

namespace   Workflow
{
    class   Helper : public QObject
    {
        Q_OBJECT

        protected: //This class is not meant to be used by itself.
            Helper( const QUuid &uuid = QUuid() );
            ~Helper();

        public:
            virtual const QUuid&    uuid() const;
            virtual qint64          begin() const = 0;
            virtual qint64          end() const = 0;
            virtual void            setBegin( qint64 begin ) = 0;
            virtual void            setEnd( qint64 end ) = 0;
            virtual qint64          length() const = 0;
            virtual void            setBoundaries( qint64 begin, qint64 end ) = 0;

        protected:
            QUuid                   m_uuid;
    };
}

Q_DECLARE_METATYPE( Workflow::Helper* )

#endif // IHELPER_H
