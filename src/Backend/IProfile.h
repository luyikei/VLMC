/*****************************************************************************
 * IProfile.h: Defines an interface of common rendering settings
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

#ifndef IPROFILE_H
#define IPROFILE_H

namespace Backend
{
    class IProfile
    {
    public:
        virtual ~IProfile() = default;

        virtual int     frameRateNum() const = 0;
        virtual int     frameRateDen() const = 0;
        virtual double  fps() const = 0;
        virtual int     width() const = 0;
        virtual int     height() const = 0;
        virtual int     aspectRatioNum() const = 0;
        virtual int     aspectRatioDen() const = 0;
        virtual double  aspectRatio() const = 0;

        virtual void    setWidth( int width ) = 0;
        virtual void    setHeight( int height ) = 0;
        virtual void    setAspectRatio( int numerator, int denominator ) = 0;
        virtual void    setFrameRate( int numerator, int denominator ) = 0;
    };
}

#endif // IPROFILE_H
