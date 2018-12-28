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

#include <stdarg.h>
#include <string.h>  // strerror_r
#ifdef _WINDOWS
#include <comdef.h>
#include <tchar.h>
#endif
#include "ckcore/assert.hh"
#include "ckcore/exception.hh"
#include "ckcore/string.hh"

namespace ckcore
{
    Exception2::Exception2(const ckcore::tchar * const err_msg)
    {
#if defined(_WINDOWS) && defined(_UNICODE)
        int utf16_size = lstrlen(err_msg) + 1;
        int utf8_size = WideCharToMultiByte(CP_UTF8,0,err_msg,utf16_size,NULL,
                                            0,NULL,NULL);

        ckASSERT(utf8_size != 0);

        err_msg_.resize(utf8_size);

        utf8_size = WideCharToMultiByte(CP_UTF8,0,err_msg,utf16_size,
                                        const_cast<char *>(err_msg_.c_str()),
                                        utf8_size,NULL,NULL);
#else
        err_msg_ = err_msg;
#endif
    }

    Exception2::Exception2(const ckcore::tstring &err_msg)
    {
#if defined(_WINDOWS) && defined(_UNICODE)
        int utf16_size = static_cast<int>(err_msg.size()) + 1;
        int utf8_size = WideCharToMultiByte(CP_UTF8,0,err_msg.c_str(),
                                            utf16_size,NULL,
                                            0,NULL,NULL);

        ckASSERT(utf8_size != 0);

        err_msg_.resize(utf8_size);

        utf8_size = WideCharToMultiByte(CP_UTF8,0,err_msg.c_str(),utf16_size,
                                        const_cast<char *>(err_msg_.c_str()),
                                        utf8_size,NULL,NULL);
#else
        err_msg_ = err_msg;
#endif
    }

    const char *Exception2::what(void) const throw()
    {
       return err_msg_.c_str();
    }

    tstring Exception2::message() const
    {
#if defined(_WINDOWS) && defined(_UNICODE)
        const int utf8_size = static_cast<int>(err_msg_.size()) + 1;

        int utf16_size = MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,
                                             err_msg_.c_str(),utf8_size,NULL,0);

        ckASSERT(utf16_size != 0);

        tstring result;
        result.resize(utf16_size + 1);

        utf16_size = MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,
                                         err_msg_.c_str(),utf8_size,
                                         const_cast<wchar_t *>(result.c_str()),
                                         utf16_size);

        return result;
#else
        return err_msg_;
#endif
    }

    ckcore::tstring get_except_msg(const std::exception &e)
    {
        const Exception2 *const ptr = dynamic_cast<const Exception2 *>(&e);
        if (ptr != NULL)
            return ptr->message();

        return ckcore::string::ansi_to_auto<1024>(e.what());
    }

    void rethrow_with_pfx(const std::exception &e,const ckcore::tchar * const fmt,...)
    {
        ckcore::tstring msg;

        va_list args;
        va_start(args,fmt);

        ckcore::string::vformatstr(msg,fmt,args);

        va_end(args);

        const ckcore::tstring err_msg = get_except_msg(e);
        msg.append(err_msg);

        throw Exception2(msg);
    }

#ifdef _WINDOWS
    void throw_from_hresult(const HRESULT res,const tchar * const pfx_fmt,...)
    {
        tstring msg;

        if (pfx_fmt != NULL)
        {
            va_list args;
            va_start(args,pfx_fmt);

            string::vformatstr(msg,pfx_fmt,args);

            va_end(args);
        }

        const tstring err_msg = _com_error(res).ErrorMessage();
        msg.append(err_msg);

        throw Exception2(msg);
    }

    // Parameter pfx_fmt can be NULL if there is no message prefix.
    static tstring build_last_error_msg(const DWORD last_err_code,const tchar * const pfx_fmt,va_list args)
    {
        // The caller should have checked whether there was a last error to collect.
        ckASSERT(last_err_code != ERROR_SUCCESS);

        tstring msg;

        if (pfx_fmt != NULL)
            ckcore::string::vformatstr(msg,pfx_fmt,args);

        tstring last_err_msg;

        // FormatMessage() does not return the size of the buffer needed. We
        // could try with a small buffer, and if that's not enough, double it
        // and so on. However, performance is not important at this point, so
        // we ask FormatMessage() to allocate a temporary buffer itself.

        LPTSTR buffer;
        DWORD len = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                                  NULL,last_err_code,0,(LPTSTR)&buffer,1,NULL);
        if (len)
        {
            try
            {
                // This can throw an exception if out of memory.
                last_err_msg = buffer;
            }
            catch (...)
            {
                ckVERIFY(NULL == LocalFree(buffer));
                throw;
            }

            ckVERIFY(NULL == LocalFree(buffer));
        }
        else
        {
            last_err_msg = ckT("<no error description available>");
        }

        msg.append(last_err_msg);
        return msg;
    }

    void throw_from_given_last_error(const DWORD last_err_code,const tchar * const pfx_fmt,...)
    {
        va_list args;
        va_start(args,pfx_fmt);
        
        tstring msg = build_last_error_msg(last_err_code,pfx_fmt,args);
        
        va_end(args);

        throw Exception2(msg);
    }

    void throw_from_last_error(const tchar * const pfx_fmt,...)
    {
        // Grab the last error as the very first thing,
        // in case something else overwrites it.
        const DWORD last_err_code = GetLastError();

        va_list args;
        va_start(args,pfx_fmt);
        
        tstring msg = build_last_error_msg(last_err_code,pfx_fmt,args);
        
        va_end(args);

        throw Exception2(msg);
    }
#endif  // #ifdef _WINDOWS

    static tstring get_errno_msg(const int errno_code)
    {
#ifdef _WINDOWS
        const tchar * const errno_msg = _tcserror(errno_code);
#else
        
        // We could write a loop here, but it's unlikely
        // that any errno error messages are so long.
        tchar errno_msg[2048];
        strerror_r(errno_code,errno_msg,sizeof(errno_msg));
#endif
        return tstring(errno_msg);
    }

    void throw_from_errno(const int errno_code,
                          const tchar * const pfx_fmt,...)
    {
        tstring msg;

        if (pfx_fmt != NULL)
        {
            va_list args;
            va_start(args,pfx_fmt);

            string::vformatstr(msg,pfx_fmt,args);

            va_end(args);
        }

        const tstring errno_msg = get_errno_msg(errno_code);
        msg.append(errno_msg);

        throw Exception2(msg);
    }

    void throw_internal_error(const tchar *file,int line)
    {
      throw Exception2(string::formatstr(ckT("internal error in %s at line %d."),file,line));
    }
}
