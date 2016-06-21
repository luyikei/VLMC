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

#include "Backend/IService.h"

namespace Mlt
{
class Service;
class Properties;
}

namespace Backend
{
class IProfile;
class IFilter;
namespace MLT
{
    class MLTService : public virtual IService
    {
    public:
        virtual ~MLTService();

        virtual std::string      identifier() const override;
        virtual IService* consumer() const override;
        virtual IService* producer() const override;
        virtual IProfile* profile() const override;
        virtual bool      attach( IFilter& filter ) override;
        virtual bool      detach( IFilter& filter ) override;
        virtual bool      detach( int index ) override;
        virtual int       filterCount() const override;
        virtual bool      moveFilter( int from, int to ) override;
        virtual IFilter*  filter( int index ) const override;
        virtual void      setProfile( IProfile& profile ) override;
        virtual bool      isValid() const override;

        Mlt::Properties*  properties();
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
