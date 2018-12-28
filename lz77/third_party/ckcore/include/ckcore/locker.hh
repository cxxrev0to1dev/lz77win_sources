/*
 * The ckCore library provides core software functionality.
 * Copyright (C) 2006-2012 Christian Kindahl
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file include/ckcore/locker.hh
 * @brief Locker class for lockable objects.
 */

#pragma once
#include "ckcore/types.hh"

namespace ckcore
{
    /**
     * @brief Template class for locking and unlocking lockable objects.
     */
    template <typename T>
    class Locker
    {
    private:
        T &lockable_;
        bool locked_;

        Locker(const Locker &rhs);
        Locker &operator=(const Locker &rhs);

    public:
        /**
         * Constructs the locker and locks the lockable object.
         * @param [in] lockable Object to lock.
         */
        explicit Locker(T &lockable) : lockable_(lockable),locked_(lockable.lock())
        {
        }

        /**
         * Destructs the locker and unlocks the lockable object.
         */
        ~Locker()
        {
            if (locked_)
                locked_ = !lockable_.unlock();
        }

        /**
         * Explicitly unlock the lockable object.
         * @return If the lockable object was successfully unlocked true is
         *         returned if not false is returned.
         */
        bool unlock()
        {
            if (!locked_)
                return false;

            locked_ = !lockable_.unlock();
            return !locked_;
        }

        /**
         * Explicitly relock the lockable object.
         * @return If the lockable object was successfully relocked true is
         *         returned if not false is returned.
         */
        bool relock()
        {
            if (locked_)
                return false;

            locked_ = lockable_.lock();
            return locked_;
        }

        /**
         * Tests if the locable object is currently locked.
         * @return If the locable is locked true is returned, if not false is
         *         returned.
         */
        bool locked()
        {
            return locked_;
        }
    };
}

