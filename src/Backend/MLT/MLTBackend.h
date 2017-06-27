/*****************************************************************************
 * MLTBackend.h:   Backend implementation consisting of
 *                 Mlt::Factory, Mlt::Profile and Mlt::Repository
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

#ifndef MLTBACKEND_H
#define MLTBACKEND_H

#include "Backend/IBackend.h"
#include "Tools/Singleton.hpp"

#include "MLTProfile.h"

namespace Mlt
{
class Repository;
class Profile;
class Output;
}

namespace Backend
{
class IOutput;
class IInput;
class IProfile;
class IClip;
class IInfo;


namespace MLT
{
class MLTBackend : public IBackend, public Singleton<MLTBackend>
{
    public:
        virtual IProfile&                   profile() override;


        virtual const std::map<std::string, IInfo*>&         availableFilters() const override;
        virtual const std::map<std::string, IInfo*>&         availableTransitions() const override;
        virtual IInfo*                                       filterInfo( const std::string& id ) const override;
        virtual IInfo*                                       transitionInfo( const std::string& id ) const override;

        virtual void            setLogHandler( LogHandler logHandler ) override;

    private:
        MLTBackend();
        ~MLTBackend();
        Mlt::Repository*    m_mltRepo;
        MLTProfile           m_profile;

        std::map<std::string, IInfo*>    m_availableFilters;
        std::map<std::string, IInfo*>    m_availableTransitions;

    friend Singleton_t::AllowInstantiation;
};

} // MLT

IBackend* instance();

}

#endif // MLTBACKEND_H
