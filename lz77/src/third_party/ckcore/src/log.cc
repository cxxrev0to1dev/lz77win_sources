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

#include <stdio.h>
#include <stdarg.h>
#include "ckcore/types.hh"
#include "ckcore/log.hh"

namespace ckcore
{
    namespace log
    {
        /**
         * Prints a message string to the logging system.
         * @param [in] format The message format or the message itself.
         */
        void def_print(const tchar *format,...)
        {
            va_list ap;
            va_start(ap,format);

#if defined(_WINDOWS) && defined(_UNICODE)
            vwprintf(format,ap);
#else
            vprintf(format,ap);
#endif
            va_end(ap);
        }

        /**
         * Prints a message string followed by a new line to the logging
         * system.
         * @param [in] format The message format or message itself.
         */
        void def_print_line(const tchar *format,...)
        {
            va_list ap;
            va_start(ap,format);

#if defined(_WINDOWS) && defined(_UNICODE)
            vwprintf(format,ap);
#else
            vprintf(format,ap);
#endif
            va_end(ap);

            printf("\n");
        }

        /**
         * Setup the default log print function.
         */
        tlog_print print = def_print;
        tlog_print print_line = def_print_line;
    }
}
