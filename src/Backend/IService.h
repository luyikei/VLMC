/*****************************************************************************
 * IService.h: Defines an interface of a common base of
 *             consumer and producer.
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

#ifndef ISERVICE_H
#define ISERVICE_H

#include <string>
#include <exception>

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

    class IService
    {
    public:
        virtual ~IService() = default;

        virtual std::string     identifier() const = 0;
        virtual IService* consumer() const = 0;
        virtual IService* producer() const = 0;
        virtual IProfile* profile() const = 0;
        virtual bool      attach( IFilter& filter ) = 0;
        virtual bool      detach( IFilter& filter ) = 0;
        virtual bool      detach( int index ) = 0;
        virtual int       filterCount() const = 0;
        // Indexes
        virtual bool      moveFilter( int from, int to ) = 0;
        virtual IFilter*  filter( int index ) const = 0;
        virtual void      setProfile( IProfile& profile ) = 0;
        virtual bool      isValid() const = 0;
    };
}

#endif // ISERVICE_H
