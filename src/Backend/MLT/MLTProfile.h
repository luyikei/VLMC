/*****************************************************************************
 * MLTProfile.h:  Wrapper of Mlt::Profile
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

#ifndef MLTPROFILE_H
#define MLTPROFILE_H

#include "Backend/IProfile.h"

namespace Mlt
{
class Repository;
class Profile;
class Output;
}

namespace Backend
{
namespace MLT
{

class MLTProfile : public IProfile
{
    public:
        MLTProfile();
        MLTProfile( Mlt::Profile* profile );
        virtual ~MLTProfile();

        virtual int     frameRateNum() const override;
        virtual int     frameRateDen() const override;
        virtual double  fps() const override;
        virtual int     width() const override;
        virtual int     height() const override;
        virtual int     aspectRatioNum() const override;
        virtual int     aspectRatioDen() const override;
        virtual double  aspectRatio() const override;

        virtual void    setWidth( int width ) override;
        virtual void    setHeight( int height ) override;
        virtual void    setAspectRatio( int numerator, int denominator ) override;
        virtual void    setFrameRate( int numerator, int denominator ) override;

    private:
        Mlt::Profile*   m_profile;

    // To access Mlt::Profile
    friend class MLTOutput;
    friend class MLTInput;
    friend class MLTBackend;
    friend class MLTTrack;
    friend class MLTMultiTrack;
    friend class MLTService;
    friend class MLTFilter;
    friend class MLTTransition;
};

}
}

#endif // MLTPROFILE_H
