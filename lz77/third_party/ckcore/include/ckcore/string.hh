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
 * @file include/ckcore/string.hh
 * @brief String helper functions.
 */

#pragma once
#include <stdio.h>
#include <stdarg.h>
#include "ckcore/types.hh"

// Not very nice since they are not in the namespace.
#ifdef _WINDOWS
#ifdef _UNICODE
//#define ckcore::string::asscanf swscanf
#define asscanf swscanf
#else
//#define ckcore::string::asscanf sscanf
#define asscanf sscanf
#endif
#else
//#define ckcore:string::asscanf sscanf
/**
 * Wrapper around sscanf function.
 * @param [in] str The input string to scan.
 * @param [in] format The input string format.
 * @return If successfull the function returns the number of arguments matched
 *         and assigned, if the function failed EOF (macro) is returned.
 */
#define asscanf sscanf
#endif

namespace ckcore
{
    namespace string
    {
        /**
         * Wrapper around the strcmp-like functions to be compatible with
         * different character encodings.
         * @param [in] str1 The first string.
         * @param [in] str2 The second string.
         * @return If str1 is less than str2 < 0 is returned, if str1 is
         *         identical to str2 0 is returned, if str1 is greater than
         *         str2 > 0 is returned.
         */
        int astrcmp(const tchar *str1,const tchar *str2);

        /**
         * Wrapper around the strncmp-like functions to be compatible with
         * different character encodings.
         * @param [in] str1 The first string.
         * @param [in] str2 The second string.
         * @param [in] n The number of characters to compare.
         * @return If str1 is less than str2 < 0 is returned, if str1 is
         *         identical to str2 0 is returned, if str1 is greater than
         *         str2 > 0 is returned.
         */
        int astrncmp(const tchar *str1,const tchar *str2,size_t n);

        /**
         * Compares two strings in a case insensitive way.
         * @param [in] str1 The first string.
         * @param [in] str2 The second string.
         * @return If str1 is less than str2 < 0 is returned, if str1 is
         *         identical to str2 0 is returned, if str1 is greater than
         *         str2 > 0 is returned.
         */
        int astrcmpi(const tchar *str1,const tchar *str2);

        /**
         * Compares two strings in a case insensitive way.
         * @param [in] str1 The first string.
         * @param [in] str2 The second string.
         * @param [in] n The number of characters to compare.
         * @return If str1 is less than str2 < 0 is returned, if str1 is
         *         identical to str2 0 is returned, if str1 is greater than
         *         str2 > 0 is returned.
         */
        int astrncmpi(const tchar *str1,const tchar *str2,size_t n);

        /**
         * Wrapper around the strlen-like functions to be compatible with
         * different character encodings.
         * @param [in] str1 The first string.
         * @return If str1 is less than str2 < 0 is returned, if str1 is
         *         identical to str2 0 is returned, if str1 is greater than
         *         str2 > 0 is returned.
         */
        size_t astrlen(const tchar *str);

        /**
         * Wrapper around the strcpy-like functions to be compatible with
         * different character encodings.
         * @param [in] str1 The first string.
         * @param [in] str2 The second string.
         * @return If str1 is less than str2 < 0 is returned, if str1 is
         *         identical to str2 0 is returned, if str1 is greater than
         *         str2 > 0 is returned.
         */
        tchar *astrcpy(tchar *str1,const tchar *str2);

        /**
         * Wrapper around the strncpy-like functions to be compatible with
         * different character encodings.
         * @param [in] str1 The first string.
         * @param [in] str2 The second string.
         * @param [in] n The number of characters to copy.
         * @return If str1 is less than str2 < 0 is returned, if str1 is
         *         identical to str2 0 is returned, if str1 is greater than
         *         str2 > 0 is returned.
         */
        tchar *astrncpy(tchar *str1,const tchar *str2,size_t n);

        /**
         * Converts an ANSI string into UTF-16 (big endian) format.
         * @param [in] ansi The ANSI string to convert.
         * @param [in] utf Pointer to buffer to which the UTF-16 string should
         *                 be written.
         * @param [in] utf_len The length of the utf buffer counted in UTF-16
         *                     characters.
         * @return The same pointer specified as the utf argument.
         */
        wchar_t *ansi_to_utf16(const char *ansi,wchar_t *utf,int utf_len);

        /**
         * Converts a UTF-16 (big endian) string into ANSI format.
         * @param [in] utf The UTF-16 string to convert.
         * @param [in] ansi Pointer to buffer to which the ANSI string should
         *                  be written.
         * @param [in] ansi_len The length of the utf buffer counted in ANSI
         *                      characters (bytes).
         * @return The same pointer specified as the ansi argument.
         */
        char *utf16_to_ansi(const wchar_t *utf,char *ansi,int ansi_len);

        /**
         * Converts an ANSI string if necessary into UTF-16 format. If UTF-16 is not
         * used the very same ANSI string is copied.
         * @param [in] ansi The ANSI string to convert.
         * @param [in] out Pointer to buffer to which the ANSI or UTF-16 string
         *                 should be written.
         * @param [in] out_len The length of the out buffer counted in UTF-16
         *                     or ANSI characters.
         * @return The same pointer specified as the out argument.
         */
        tchar *ansi_to_auto(const char *ansi,tchar *out,int out_len);

        /**
         * Creates a formatted string from a format description similar to that
         * of sprintf. This routine accept positional arguments like "%1$s".
         * @param [out] res Output formatted string.
         * @param [in] fmt The string format.
         * @param [in] args Variable argument list.
         */
        void vformatstr(tstring &res,const tchar * fmt,
                        va_list args);

        /**
         * Creates a formatted string from a format description similar to that
         * of sprintf. This routine accept positional arguments like "%1$s".
         * @param [in] fmt The string format.
         * @return The resulting formatted string.
         */
        tstring formatstr(const tchar * fmt,...) __attribute__ ((format (printf, 1, 2)));

        /**
         * Converts an ANSI string into the internal string format.
         * @param [in] str String to convert.
         * @return str in internal string format.
         */
        tstring to_auto(const char *str);

        /**
         * Converts an ANSI string into the internal string format.
         * @param [in] str String to convert.
         * @return str in internal string format.
         */
        tstring to_auto(const std::string &str);

        /**
         * Converts an ANSI string if necessary into UTF-16 format. If UTF-16 is not
         * used the very same ANSI string is returned.
         * @param [in] str The ANSI string to return and maybe convert.
         * @return The specified ANSI string either in ANSI or in UTF-16 format.
         */
        template<size_t S>
        tstring ansi_to_auto(const char *str)
        {
#if defined(_WINDOWS) && defined(_UNICODE)
            tchar res[S];
            return tstring(ansi_to_utf16(str,res,sizeof(res)/sizeof(tchar)));
#else
            return tstring(str);
#endif
        }

        /**
         * Converts an ANSI or an UTF-16 string (depending on compilation options)
         * into ANSI format.
         * @param [in] str The ANSI or UTF-16 string to convert.
         * @return The specified string converted into ANSI format.
         */
        template<size_t S>
        std::string auto_to_ansi(const tchar *str)
        {
#if defined(_WINDOWS) && defined(_UNICODE)
            char res[S];
            return std::string(utf16_to_ansi(str,res,sizeof(res)));
#else
            return std::string(str);
#endif
        }

        /**
         * Converts an UTF-16 string if necessary into ANSI format. If ANSI is not
         * used the very same UTF-16 string is returned.
         * @param [in] str The UTF-16 string to return and maybe convert.
         * @return The specified UTF-16 string either in ANSI or in UTF-16 format.
         */
        template<size_t S>
        tstring utf16_to_auto(const wchar_t *str)
        {
#if defined(_WINDOWS) && defined(_UNICODE)
            return tstring(str);
#else
            tchar res[S];
            return tstring(utf16_to_ansi(str,res,sizeof(res)/sizeof(tchar)));
#endif
        }
    }
}
