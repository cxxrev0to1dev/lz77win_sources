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
 * @file include/ckcore/log.hh
 * @brief Defines a data logging interface.
 */

#pragma once
#include "ckcore/types.hh"

namespace ckcore
{
    /**
     * Global log name space.
     */
    namespace log
    {
        /**
         * Defines the print function type.
         */
        typedef void (*tlog_print)(const tchar *format,...) __attribute__ ((format (printf, 1, 2)));

        extern tlog_print print;
        extern tlog_print print_line;
    }

    /**
     * @brief Interface for data logging.
     */
    class Log
    {
    public:
        virtual ~Log() {};

        /**
         * Prints a message string to the logging system.
         * @param [in] format The message format or the message itself.
         */
        virtual void print(const tchar *format,...) __attribute__ ((format (printf, 2, 3))) = 0;

        /**
         * Prints a message string followed by a new line to the logging
         * system.
         * @param [in] format The message format or message itself.
         */
        virtual void print_line(const tchar *format,...) __attribute__ ((format (printf, 2, 3))) = 0;
    };
}

