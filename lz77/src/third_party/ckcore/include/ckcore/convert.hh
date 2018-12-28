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
 * @file include/ckcore/convert.hh
 * @brief Type conversion functions.
 */

#pragma once
#include "ckcore/types.hh"

namespace ckcore
{
    namespace convert
    {
        /**
         * A safe platform intependant version of the sprintf function.
         * @param [in] buffer The target string buffer.
         * @param [in] size The size of the buffer in bytes.
         * @param [in] format Format string.
         */
        void sprintf(tchar *buffer,size_t size, const tchar *format, ...) __attribute__ ((format (printf, 3, 4)));

        enum { INT_TO_STR_BUFLEN = 30 };  // more than enough for 64-bit integers, used by b_to_str2() too.

        /**
         * Converts the specified boolean value into a string.
         */
        void b_to_str2(bool value, tchar * buffer);

        /**
         * Most implementations of itoa() and printf() lock the locale settings
         * and are therefore much slower than this routine.
         */
        void ui64_to_str2(tuint64 value, tchar * buffer);

        inline void i64_to_str2(tint64 value, tchar * buffer)
        {
          if (value >= 0)
          {
            ui64_to_str2(value, buffer);
          }
          else
          {
            *buffer = '-';
            ui64_to_str2(tuint64(-value), buffer + 1);
          }
        }

        inline void i32_to_str2(tint32 value, tchar * buffer)
        {
          i64_to_str2(value, buffer);
        }

        inline void ui32_to_str2(tuint32 value, tchar * buffer)
        {
          ui64_to_str2(value, buffer);
        }

        /**
         * Converts the specified big endian unsigned 32-bit integer value into
         * a little endian 32-bit unsigned integer value.
         * @param [in] value The integer value to convert.
         * @return The converted integer value.
         */
        tuint32 be_to_le32(tuint32 value);

        /**
         * Converts the specified big endian unsigned 16-bit integer value into
         * a little endian 16-bit unsigned integer value.
         * @param [in] value The integer value to convert.
         * @return The converted integer value.
         */
        tuint16 be_to_le16(tuint16 value);

        /**
         * Converts a tm structure into DOS date and time format.
         * @param [in] tm The input time structure.
         * @param [out] dos_date The date in DOS format.
         * @param [out] dos_time The time in DOS format.
         */
        void tm_to_dostime(struct tm &time, unsigned short &dos_date,
                           unsigned short &dos_time);
    }
}
