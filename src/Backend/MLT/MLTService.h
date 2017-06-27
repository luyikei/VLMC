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

#include "Backend/IInfo.h"

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

    class MLTServiceInfo : public IInfo
    {
    public:
        MLTServiceInfo() = default;
        virtual ~MLTServiceInfo() override;

        virtual const std::string&  identifier() const override;
        virtual const std::string&  name() const override;
        virtual const std::string&  description() const override;
        virtual const std::string&  author() const override;
        virtual const std::vector<IParameterInfo*>& paramInfos() const override;

        void                        setProperties( Mlt::Properties* properties );

    private:
        std::string             m_identifier;
        std::string             m_name;
        std::string             m_author;
        std::string             m_description;

        std::vector<IParameterInfo*> m_paramInfos;
    };

    class MLTService
    {
    public:
        virtual ~MLTService();

        virtual Mlt::Service*   service();
        virtual Mlt::Service*   service() const;

        virtual std::string     identifier() const;
        bool                    isValid() const;
        Mlt::Properties*        properties();

    private:
        Mlt::Service*     m_service;
    };
}
}

#endif // MLTSERVICE_H
