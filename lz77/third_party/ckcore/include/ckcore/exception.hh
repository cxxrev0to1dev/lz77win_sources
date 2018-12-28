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
 * @file include/ckcore/exception.hh
 * @brief Defines the exception base class.
 */

#pragma once

#ifdef _WINDOWS
#include <windows.h>
#endif

#include "ckcore/types.hh"
#include "ckcore/string.hh"

namespace ckcore
{
    /**
     * @brief Exception class.
     */
    class Exception2 : public std::exception
    {
    private:
        std::string err_msg_;

    public:
        /**
         * Construct an Exception object.
         * @param [in] err_msg The error message.
         */
        Exception2(const tchar * const err_msg);

        /**
         * Construct an Exception object.
         * @param [in] err_msg The error message.
         */
        Exception2(const tstring &err_msg);
        virtual ~Exception2() throw() {};

        /**
         * Returns the error message in UTF-8 format.
         * @return The error message.
         */
        virtual const char *what(void) const throw();

        /**
         * Returns the error message in tstring format.
         * @return The error message.
         */
        tstring message() const;
    };

    /**
     * Returns the exception message of the given exception object. The
     * function tests if the exception is an Exception2 and in that case
     * returns its message.
     * @param [in] e The exception object.
     * @return The error message of the exception.
     */
    tstring get_except_msg(const std::exception &e);

    /**
     * Rethrows the exception with a prefix message.
     * @param [in] e The exception to rethrow.
     * @param [in] szFormatStr The message format string.
     */
    void MSC_DECLSPEC_NORETURN rethrow_with_pfx(const std::exception &e,const tchar *fmt,...) __attribute__ ((noreturn, format (printf, 2, 3)));

#ifdef _WINDOWS
    void MSC_DECLSPEC_NORETURN throw_from_hresult(HRESULT res,const tchar * pfx_fmt,...);
    void MSC_DECLSPEC_NORETURN throw_from_last_error(const tchar * pfx_fmt,...);
    // Sometimes the caller needs to clean something up before calling throw_from_last_error(),
    // and that can reset the last error, so we need another version of this routine with a
    // manually-stored error code.
    void MSC_DECLSPEC_NORETURN throw_from_given_last_error(DWORD lastErrorCode, const tchar * pfx_fmt,...);
#endif
    void MSC_DECLSPEC_NORETURN throw_from_errno(int errno_code,const tchar * pfx_fmt,... ) __attribute__ ((noreturn, format (printf, 2, 3)));
    void MSC_DECLSPEC_NORETURN throw_internal_error(const tchar *file,int line) __attribute__ ((noreturn));
};  // namespace ckcore
