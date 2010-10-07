/*****************************************************************************
 * Helper.h: Describes a common interface for all workflow helpers.
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
            Helper( qint64 begin = 0, qint64 end = -1, const QString &uuid = QString() );

        public:
            virtual const QUuid&    uuid() const;
            virtual qint64          begin() const;
            virtual qint64          end() const;
            virtual void            setBegin( qint64 begin );
            virtual void            setEnd( qint64 end );
            virtual qint64          length() const;
            virtual void            setBoundaries( qint64 begin, qint64 end );

        protected:
            qint64                  m_begin;
            qint64                  m_end;
            QUuid                   m_uuid;

        signals:
            void                    lengthUpdated();
            void                    destroyed( const QUuid &uuid );
    };
}

Q_DECLARE_METATYPE( Workflow::Helper* );

#endif // IHELPER_H
