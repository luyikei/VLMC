/*****************************************************************************
 * Singleton.hpp : Generic singleton pattern implementation
 *****************************************************************************
 * Copyright (C) 2008-2016 VideoLAN
 *
 * Authors: Hugo Beauzée-Luyssen <hugo@beauzee.fr>
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


/** \file
  * This file contain the templated singleton.
  * Class/struct to be singletonized just have to
  * iherit from this classe with the class/struct type in template
  * parameter.
  */

#ifndef SINGLETON_HPP
#define SINGLETON_HPP

#include <stdlib.h>

template <typename T>
class       Singleton
{
public:
    static T*      instance()
    {
        if ( m_instance == nullptr )
            m_instance = new T;
        return m_instance;
    }

    static void    destroyInstance()
    {
        if ( m_instance != nullptr )
        {
            delete m_instance;
            m_instance = nullptr;
        }
    }
protected:
    Singleton(){}
    ~Singleton() = default;
    Singleton(const Singleton<T>&) = delete;
    Singleton(Singleton<T>&&) = delete;
    Singleton<T>&   operator=(const Singleton<T>&) = delete;
    Singleton<T>&   operator=(Singleton<T>&&) = delete;

protected:
    static T*      m_instance;
};

template <typename T>
T*  Singleton<T>::m_instance = nullptr;

#endif // SINGLETON_HPP
