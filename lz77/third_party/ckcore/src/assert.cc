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

#include <iostream>
#ifdef _WINDOWS
#include <windows.h>
#endif
#include "ckcore/assert.hh"
#include "ckcore/string.hh"

namespace ckcore
{
    void dbg_printf(const char * format, ...)
    {
        va_list args;
        va_start(args,format);

        char buffer[ckDBG_PRINT_BUF_SIZE];
#ifdef _WINDOWS
        if(_vsnprintf(buffer,ckDBG_PRINT_BUF_SIZE - 1,format,args) < 0)
            buffer[ckDBG_PRINT_BUF_SIZE - 1] = '\0';

        OutputDebugStringA(buffer);
#else
        if(vsnprintf(buffer,ckDBG_PRINT_BUF_SIZE - 1,format,args) < 0)
            buffer[ckDBG_PRINT_BUF_SIZE - 1] = '\0';

        std::cerr << buffer;
#endif
		va_end(args);
    }

    void dbg_trace_printf(const char * file,int line,const char * format, ...)
    {
        std::stringstream full_fmt;
        full_fmt << "[" << file << ":" << line << "] " << format;

        va_list args;
        va_start(args,format);

        char buffer[ckDBG_PRINT_BUF_SIZE];
#ifdef _WINDOWS
        if(_vsnprintf(buffer,ckDBG_PRINT_BUF_SIZE - 1,full_fmt.str().c_str(),args) < 0)
            buffer[ckDBG_PRINT_BUF_SIZE - 1] = '\0';

        OutputDebugStringA(buffer);
#else
        if(vsnprintf(buffer,ckDBG_PRINT_BUF_SIZE - 1,full_fmt.str().c_str(),args) < 0)
            buffer[ckDBG_PRINT_BUF_SIZE - 1] = '\0';

        std::cerr << buffer;
#endif
		va_end(args);
    }
}
