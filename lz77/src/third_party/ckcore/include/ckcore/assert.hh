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
 * @file include/ckcore/assert.hh
 * @brief Assertion support.
 */

#pragma once
#if defined(_WINDOWS)
#  include <crtdbg.h>
#elif defined(_UNIX)
#  include <cassert>
#endif
#include "ckcore/types.hh"

#define ckDBG_PRINT_BUF_SIZE        1024

namespace ckcore
{
    /**
     * Formats and prints a string to the debug output system. On Windows this
     * is the debugger display log and on other systems it's standard error.
     * @param [in] format Message format.
     */
    void dbg_printf(const char * format, ...);

    /**
     * Formats and prints a string to the debug output system. On Windows this
     * is the debugger display log and on other systems it's standard error.
     * @param [in] file Source file sending the message.
     * @param [in] line Line in source file sending the message.
     * @param [in] format Message format.
     */
    void dbg_trace_printf(const char * file,int line,const char * format, ...);
}

#ifndef ckASSERT
#  ifdef _DEBUG
#    ifdef _WINDOWS
#      define ckASSERT(expr) _ASSERTE(expr)
#    else
#      define ckASSERT(expr) assert(expr)
#    endif
#  else
#    define ckASSERT(ignore) ((void)0)
#  endif
#endif

#ifndef ckVERIFY
#  ifdef _DEBUG
#    define ckVERIFY(expr) ckASSERT(expr)
#  else
#    define ckVERIFY(expr) (expr)
#  endif
#endif

#ifndef ckDEBUG
#  ifdef _DEBUG
#    define ckDEBUG(expr) (expr)
#  else
#    define ckDEBUG(ignore) ((void)0)
#  endif
#endif

#ifndef ckTRACE
#  ifdef _DEBUG
#    define ckTRACE(format,...) dbg_trace_printf(__FILE__,__LINE__,format,__VA_ARGS__)
#  else
#    define ckTRACE(format,...) ((void)0)
#  endif
#endif

#ifndef ckTRACE_IF
#  ifdef _DEBUG
#    define ckTRACE_IF(cond,format,...) do { if (cond) { dbg_trace_printf(__FILE__,__LINE__,format,__VA_ARGS__); } } while (0)
#  else
#    define ckTRACE_IF(cond,format,...) ((void)0)
#  endif
#endif
