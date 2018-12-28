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
#include <cassert>
#include <cstring>
#ifdef _WINDOWS
#include <windows.h>
#include <atlbase.h>
//#include <atlapp.h>	// Isn't included in VS2010 installation.
#endif
#include "ckcore/assert.hh"
#include "ckcore/string.hh"

namespace ckcore
{
    namespace string
    {
        int astrcmp(const tchar *str1,const tchar *str2)
        {
#if defined(_WINDOWS) && defined(_UNICODE)
            return wcscmp(str1,str2);
#else
            return strcmp(str1,str2);
#endif
        }

        int astrncmp(const tchar *str1,const tchar *str2,size_t n)
        {
#if defined(_WINDOWS) && defined(_UNICODE)
            return wcsncmp(str1,str2,n);
#else
            return strncmp(str1,str2,n);
#endif
        }

        int astrcmpi(const tchar *str1,const tchar *str2)
        {
#ifdef _WINDOWS
#ifdef _UNICODE
            return _wcsicmp(str1,str2);
#else
            return _stricmp(str1,str2);
#endif
#else
            return strcasecmp(str1,str2);
#endif
        }

        int astrncmpi(const tchar *str1,const tchar *str2,size_t n)
        {
#ifdef _WINDOWS
#ifdef _UNICODE
            return _wcsnicmp(str1,str2,n);
#else
            return _strnicmp(str1,str2,n);
#endif
#else
            return strncasecmp(str1,str2,n);
#endif
        }

        size_t astrlen(const tchar *str)
        {
#if defined(_WINDOWS) && defined(_UNICODE)
            return wcslen(str);
#else
            return strlen(str);
#endif
        }

        tchar *astrcpy(tchar *str1,const tchar *str2)
        {
#if defined(_WINDOWS) && defined(_UNICODE)
            return wcscpy(str1,str2);
#else
            return strcpy(str1,str2);
#endif
        }

        tchar *astrncpy(tchar *str1,const tchar *str2,size_t n)
        {
#if defined(_WINDOWS) && defined(_UNICODE)
            return wcsncpy(str1,str2,n);
#else
            return strncpy(str1,str2,n);
#endif
        }

        void vformatstr(tstring &res,const tchar * const fmt,
                        va_list args)
        {
#ifdef _WINDOWS
            va_list args_copy = args;

            // There are here several ways to write this routine:
            // 1) Call vasprintf(), insert the result into the string, free()
            //    the result.
            // 2) [the one implemented] Call Microsoft's _vscprintf() to
            //    calculate the string length upfront, make enough room for
            //    that length, then call sprintf() to print the string.
            // 3) Try once with snprintf(), and, if not enough room was
            //    available, make enough room and call snprintf() again.
            // 4) Try once with Microsoft's _snprintf(), and, if not enough
            //    room, increase room (perhaps exponentially) and try again.
            const int char_cnt = _vsctprintf_p(fmt,args);
            if (char_cnt == 0)
            {
                res.clear();
                return;
            }

            res.resize(char_cnt + 1);
            ckVERIFY(char_cnt == _vstprintf_p(&res[0],char_cnt + 1,fmt,args_copy));

            // Remove the null terminator, std::string will add its own if
            // necessary.
            res.resize(char_cnt);

#else  // #ifdef _WINDOWS
            va_list args_copy;
            va_copy(args_copy,args);

            const int char_cnt = vsnprintf(NULL,0,fmt,args);
            if (char_cnt == 0)
            {
                res.clear();
                return;
            }

            res.resize(char_cnt + 1);
            if (char_cnt != vsnprintf(&res[0],char_cnt + 1,fmt,args_copy))
                 assert(false);

            // Remove the null terminator, std::string will add its own if
            // necessary.
            res.resize(char_cnt);
#endif  // #ifdef _WINDOWS
        }

        tstring formatstr(const tchar * const fmt,...)
        {
            tstring res;
            
            va_list args;
            va_start(args,fmt);
            
            vformatstr(res,fmt,args);
            
            va_end(args);
            return res;
        }

        wchar_t *ansi_to_utf16(const char *ansi,wchar_t *utf,int utf_len)
        {
            ckASSERT(utf_len >= 0);     // See size_t typecast below.
#ifdef _WINDOWS
            int converted = MultiByteToWideChar(AreFileApisANSI() ? CP_ACP : CP_OEMCP,
                                                MB_PRECOMPOSED,ansi,(int)strlen(ansi) + 1,
                                                utf,utf_len);

            // Truncate UTF-16 string if buffer is too small.
            if (converted == utf_len)
                utf[utf_len - 1] = '\0';
#else
            size_t ansi_len = strlen(ansi);
            size_t out_len = ansi_len >= size_t(utf_len) ? utf_len - 1 : ansi_len;

            for (size_t i = 0; i < out_len; i++)
                utf[i] = ansi[i];

            utf[out_len] = '\0';
#endif
            return utf;
        }

        char *utf16_to_ansi(const wchar_t *utf,char *ansi,int ansi_len)
        {
            ckASSERT(ansi_len >= 0);    // See size_t typecast below.
#ifdef _WINDOWS
            int converted = WideCharToMultiByte(AreFileApisANSI() ? CP_ACP : CP_OEMCP,0,
                utf,(int)lstrlenW(utf) + 1,ansi,ansi_len,NULL,NULL);

            // Truncate the ANSI string of buffer is too small.
            if (converted == ansi_len)
                ansi[ansi_len - 1] = '\0';
#else
            size_t utf_len = wcslen(utf);
            size_t out_len = utf_len >= size_t(ansi_len) ? ansi_len - 1 : utf_len;

            for (size_t i = 0; i < out_len; i++)
                ansi[i] = utf[i] & 0xff;

            ansi[out_len] = '\0';
#endif
            return ansi;
        }

        tchar *ansi_to_auto(const char *ansi,tchar *out,int out_len)
        {
#if defined(_WINDOWS) && defined(_UNICODE)
            return ansi_to_utf16(ansi,out,out_len);
#else
            return strncpy(out,ansi,out_len);
#endif
        }

        tstring to_auto(const char *str)
        {
#if defined(_WINDOWS) && defined(_UNICODE)
            int needed = MultiByteToWideChar(AreFileApisANSI() ? CP_ACP : CP_OEMCP,
                                             MB_PRECOMPOSED, str, -1,
                                             NULL, 0);
            assert(needed > 0);
            if (needed == 0)
                return ckT("");

            tstring res(needed - 1, ' ');
            if (MultiByteToWideChar(AreFileApisANSI() ? CP_ACP : CP_OEMCP,
                                    MB_PRECOMPOSED, str, -1,
                                    const_cast<wchar_t *>(res.c_str()), needed) == 0)
            {
                assert(false);
                return ckT("");
            }

            return res;
#else
            return str;
#endif
        }

        tstring to_auto(const std::string &str)
        {
            return to_auto(str.c_str());
        }
    }
}
