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
 * @file include/ckcore/system.hh
 * @brief Defines the system namespace.
 */

#pragma once
#include "ckcore/types.hh"

namespace ckcore
{
    namespace system
    {
        /**
         * Defines different cache levels.
         */
        enum CacheLevel
        {
            ckLEVEL_1 = 0x01,
            ckLEVEL_2,
            ckLEVEL_3
        };

        /**
         * Returns the number of milliseconds that has elapsed since the system
         * was started.
         * @return The number of milliseconds since the system was started.
         */
        tuint64 time();

        /**
         * Returns the number of clock cycles executed by the host processor
         * since the system was started.
         * @return The number of executed clock cycles since the system was
         *         started.
         */
        tuint64 ticks();

        /**
         * Determines the size of the specified cache. This function will only
         * be able to obtain the cache sizes on AMD and Intel systems.
         * @return If successfull the size of the cache is returned in bytes,
         *         if unsuccessfull 0 is returned.
         */
        unsigned long cache_size(CacheLevel level);
    }
}

