/*****************************************************************************
 * MLTService.h:  Wrapper of Mlt::Service
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

#ifndef MLTSERVICE_H
#define MLTSERVICE_H

#include <memory>
#include <exception>

namespace Mlt
{
class Service;
class Properties;
}

namespace Backend
{
class IProfile;
class IFilter;

class InvalidServiceException : public std::exception
{
public:
    virtual const char* what() const throw() override
    {
        return "We couldn't create a service.";
    }
};

namespace MLT
{
    class MLTService
    {
    public:
        virtual ~MLTService();

        virtual std::string     identifier() const;
        bool                    isValid() const;
        Mlt::Properties*        properties();
    protected:
        // Not intended to be created.
        MLTService();
        MLTService( Mlt::Service* service );


        Mlt::Service*     m_service;

    friend class MLTFilter;
    };
}
}

#endif // MLTSERVICE_H
