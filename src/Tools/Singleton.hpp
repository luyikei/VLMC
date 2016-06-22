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

#include <memory>
#include <mutex>

namespace details
{
namespace policy
{
template <typename T>
struct OldPolicy
{
    static T* get()
    {
        if (s_instance == nullptr)
            s_instance = new T;
        return s_instance;
    }

    static void release()
    {
        delete s_instance;
        s_instance = nullptr;
    }

    using InstanceType = T*;

private:
    static T* s_instance;
};

template <typename T>
T* OldPolicy<T>::s_instance = nullptr;

template <typename T>
struct MeyersPolicy
{
    static T* get()
    {
        static T inst;
        return &inst;
    }
    // No release method; automatically released when program exits
};

template <typename T>
class ScopedPolicy
{
    struct Lock
    {
        std::shared_ptr<T> ptr;
        friend class ScopedPolicy;
    };

public:
    static Lock lock()
    {
        auto inst = s_weak_inst.lock();
        if ( inst )
            return Lock{ inst };
        std::unique_lock<std::mutex> lock( s_mutex );
        inst = s_weak_inst.lock();
        if ( !inst )
        {
            // use a lambda expression since the shared_ptr doesn't have access to T::~T()
            s_weak_inst = inst = std::shared_ptr<T>( new T, [](T*ptr){ delete ptr; } );
        }
        return Lock{ inst };
    }

    static std::shared_ptr<T> get()
    {
        return lock().ptr;
    }

    static std::mutex s_mutex;
    static std::weak_ptr<T> s_weak_inst;
};

template <typename T>
std::mutex ScopedPolicy<T>::s_mutex;
template <typename T>
std::weak_ptr<T> ScopedPolicy<T>::s_weak_inst;
}

template <typename T, typename Policy>
class       PSingleton
{
public:
    using Singleton_t = PSingleton<T, Policy>;
    /// Meant to be used by Singleton<> users to declare the Policy as a friend class.
    /// The AllowInstantiation just reads better than Singleton<T>::Policy_t
    using AllowInstantiation = Policy;
    using Policy_t = Policy;

    static auto      instance() -> decltype(Policy::get())
    {
        return Policy::get();
    }

    static void    destroyInstance()
    {
        Policy::release();
    }

protected:
    PSingleton(){}
    ~PSingleton() = default;

public:
    PSingleton(const Singleton_t&) = delete;
    PSingleton(Singleton_t&&) = delete;
    Singleton_t&   operator=(const Singleton_t&) = delete;
    Singleton_t&   operator=(Singleton_t&&) = delete;
};

}

/**
 * @brief Soingleton defines the "old" way of having a singleton.
 * ie. by having to call instance()/destroyInstance() couple
 * @warning This isn't thread safe
 */
template <typename T>
using Singleton = details::PSingleton<T, details::policy::OldPolicy<T>>;

/**
 * @brief MeyersPolicy defines the usual C++11 singleton
 * This is threadsafe, but doesn't let you control the moment the instance will be
 * released
 */
template <typename T>
using MeyersSingleton = details::PSingleton<T, details::policy::MeyersPolicy<T>>;

/**
 * @brief Defined a singleton that will be released when the originally acquied "lock"
 * falls out of scope
 * Inherithing from this singleton implementation will provide a T::Policy_t::lock() method
 * which needs to be called to acquire a lock.
 * When this lock falls out of scope, any subsequent call to lock()/instance() will create a new instance.
 * Calling instance() without having a lock instantiated will create a new instance that will get destroyed
 * when the returned instance falls out of scope.
 */
template <typename T>
using ScopedSingleton = details::PSingleton<T, details::policy::ScopedPolicy<T>>;


#endif // SINGLETON_HPP
