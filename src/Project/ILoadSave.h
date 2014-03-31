/*****************************************************************************
 * ILoadSave.h: Defines the basic interaction for a component that wishes to
 * load or save its state to a project
 *****************************************************************************
 * Copyright (C) 2008-2014 VideoLAN
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

#ifndef ILOADSAVE_H
#define ILOADSAVE_H

class QDomDocument;
class QXmlStreamWriter;

class ILoadSave
{
    public:
        virtual ~ILoadSave() {}
        virtual bool    load( const QDomDocument& project ) = 0;
        virtual bool    save( QXmlStreamWriter& project ) = 0;
};

#endif // ILOADSAVE_H
