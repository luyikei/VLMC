/*****************************************************************************
 * MLTParameterInfo.h:  Parameter information
 *****************************************************************************
 * Copyright (C) 2008-2017 VideoLAN
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

#ifndef MLTPARAMETERINFO_H
#define MLTPARAMETERINFO_H

#include "Backend/IParameterInfo.h"

namespace Mlt
{
class Properties;
}

namespace Backend
{
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
}
}

#endif // MLTPARAMETERINFO_H
