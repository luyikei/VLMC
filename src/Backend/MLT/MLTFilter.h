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
    class MLTParameterInfo : public IParameterInfo
    {
    public:
        MLTParameterInfo() = default;
        virtual const std::string&  identifier() const override;
        virtual const std::string&  name() const override;
        virtual const std::string&  type() const override;
        virtual const std::string&  description() const override;

        virtual const std::string&  defaultValue() const override;
        virtual const std::string&  minValue() const override;
        virtual const std::string&  maxValue() const override;

        void                        setProperties( Mlt::Properties* properties );

    private:
        std::string             m_identifier;
        std::string             m_name;
        std::string             m_type;
        std::string             m_description;
        std::string             m_defaultValue;
        std::string             m_minValue;
        std::string             m_maxValue;
    };

    class MLTFilterInfo : public IFilterInfo
    {
    public:
        MLTFilterInfo() = default;
        virtual ~MLTFilterInfo() override;

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

    class MLTFilter : public IFilter, public MLTService
    {
    public:
        MLTFilter( IProfile& profile, const char* id );
        MLTFilter( const char* id );
        MLTFilter( Mlt::Filter* filter, Mlt::Producer* connectedProducer );
        virtual ~MLTFilter() override;

        virtual std::string     identifier() const override;
        virtual bool    connect( IInput& input, int index = 0 ) override;
        virtual void    setBoundaries( int64_t begin, int64_t end ) override;
        virtual int64_t begin() const override;
        virtual int64_t end() const override;
        virtual int64_t length() const override;

        virtual std::shared_ptr<IInput> input() const override;

        virtual const IFilterInfo&  filterInfo() const override;

    private:
        Mlt::Filter*        m_filter;
        Mlt::Producer*      m_connectedProducer;

    friend class MLTMultiTrack;
    friend class MLTInput;
    };
}
}

#endif // MLTFILTER_H
