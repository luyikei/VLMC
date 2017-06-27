/*****************************************************************************
 * MLTFilter.h:  Wrapper of Mlt::Filter
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

#ifndef MLTFILTER_H
#define MLTFILTER_H

#include "Backend/IFilter.h"
#include "MLTService.h"

namespace Mlt
{
class Filter;
class Properties;
class Producer;
}

namespace Backend
{
class IProfile;
class IInput;
namespace MLT
{
    class MLTFilter : public IFilter, public MLTService
    {
    public:
        MLTFilter( IProfile& profile, const char* id );
        MLTFilter( const char* id );
        MLTFilter( Mlt::Filter* filter, Mlt::Producer* connectedProducer );
        virtual ~MLTFilter() override;

        virtual Mlt::Filter*    filter();
        virtual Mlt::Filter*    filter() const;

        virtual Mlt::Service*   service() override;
        virtual Mlt::Service*   service() const override;

        virtual std::string     identifier() const override;
        virtual bool    connect( IInput& input, int index = 0 ) override;
        virtual void    setBoundaries( int64_t begin, int64_t end ) override;
        virtual int64_t begin() const override;
        virtual int64_t end() const override;
        virtual int64_t length() const override;
        virtual void    detach() override;

        virtual std::shared_ptr<IInput> input() const override;

        virtual const IInfo&  filterInfo() const override;

    private:
        Mlt::Filter*        m_filter;
        std::unique_ptr<Mlt::Producer>      m_connectedProducer;
    };
}
}

#endif // MLTFILTER_H
