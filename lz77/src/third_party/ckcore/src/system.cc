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

#include <string.h>

#if defined(_WINDOWS)
#include <windows.h>
#include <intrin.h>
#elif defined(_UNIX)
#include <sys/time.h>
#else
#error "Unknown platform."
#endif

#include "ckcore/system.hh"

namespace ckcore
{
    namespace system
    {
        tuint64 time()
        {
#ifdef _WINDOWS
            return GetTickCount();
#else
            struct timeval time;
            gettimeofday(&time,(struct timezone *)0);
            return (tuint64)(time.tv_sec * 1000 + (time.tv_usec / 1000));
#endif
        }

        tuint64 ticks()
        {
#ifdef _WINDOWS
            return __rdtsc();
#else
            unsigned long low = 0,high = 0;

            asm volatile("rdtsc"
                :"=a"(low),"=d"(high)
                :
                :);

            return ((tuint64)high << 32) | low;
#endif
        }

#if defined(_WINDOWS) && defined(_M_X64)
        extern "C" void cpuid64(unsigned long func,unsigned long arg,unsigned long *words);
#endif

        void cpuid(unsigned long func,unsigned long arg,
                   unsigned long &a,unsigned long &b,
                   unsigned long &c,unsigned long &d)
        {
#ifdef _WINDOWS
#ifdef _M_X64
            unsigned long words[4];
            cpuid64(func,arg,words);

            a = words[0];
            b = words[1];
            c = words[2];
            d = words[3];
#else
            // I don't know how to copy back to arguments passed by reference, that's
            // why I use temporary variables first.
            unsigned long t1,t2,t3,t4;

            __asm
            {
                mov eax,func
                mov ecx,arg
                cpuid
                mov dword ptr t1,eax
                mov dword ptr t2,ebx
                mov dword ptr t3,ecx
                mov dword ptr t4,edx
            };

            a = t1;
            b = t2;
            c = t3;
            d = t4;
#endif
#else
            // 32-bit PIC compatible version.
#if defined(__i386__) && defined(__PIC__)
            asm("pushl %%ebx\n"
                "cpuid\n"
                "movl %%ebx,%1\n"
                "popl %%ebx\n"
                :"=a"(a),
                 "=r"(b),
                 "=c"(c),
                 "=d"(d)
                :"a"(func),
                 "c"(arg));
#else
            asm("cpuid"
                :"=a"(a),
                 "=b"(b),
                 "=c"(c),
                 "=d"(d)
                :"a"(func),
                 "c"(arg));
#endif
#endif
        }

        /**
         * Determines the size of the specified cache. This function will only
         * be able to obtain the cache sizes on Intel systems.
         * @return If successfull the size of the cache is returned in bytes,
         *         if unsuccessfull 0 is returned.
         */
        unsigned long cache_size_intel(CacheLevel level)
        {
            unsigned long reg = 0;
            while (true)
            {
                unsigned long a,b,c,d;
                cpuid(4,reg++,a,b,c,d);

                // Check if we have found the last cache.
                unsigned char cur_type = (unsigned char)a & 0x1f;
                if (cur_type == 0)
                    break;

                // We're only interested in the level 1 data cache.
                unsigned char cur_level = (unsigned char)(a >> 5) & 0x07;
                if ((cur_type == 1 || cur_type == 3) && cur_level == level)
                {
                    unsigned long ways = (b >> 22) & 0x3ff;
                    unsigned long part = (b >> 12) & 0x3ff;
                    unsigned long line = b & 0xfff;
                    unsigned long sets = c;

                    return (ways + 1) * (part + 1) * (line + 1) * (sets + 1);
                }
            }

            return 0;
        }

        /**
         * Determines the size of the specified cache. This function will only
         * be able to obtain the cache sizes on AMD systems.
         * @return If successfull the size of the cache is returned in bytes,
         *         if unsuccessfull 0 is returned.
         */
        unsigned long cache_size_amd(CacheLevel level)
        {
            unsigned long a,b,c,d;

            if (level == ckLEVEL_1)
            {
                cpuid(0x80000005,0,a,b,c,d);
                return ((c >> 24) & 0xff) * 1024;
            }
            else if (level == ckLEVEL_2)
            {
                cpuid(0x80000006,0,a,b,c,d);
                return ((c >> 16) & 0xffff) * 1024;
            }

            // Level 3 can not be determined exactly.
            return 0;
        }

        unsigned long cache_size(CacheLevel level)
        {
            // Obtain processor vendor identifier.
            unsigned long a,b,c,d;
            cpuid(0,0,a,b,c,d);

            char vendor[13];
            memcpy(vendor    ,&b,4);
            memcpy(vendor + 4,&d,4);
            memcpy(vendor + 8,&c,4);
            vendor[12] = '\0';

            if (!strcmp(vendor,"GenuineIntel"))
                return cache_size_intel(level);
            else if (!strcmp(vendor,"AuthenticAMD"))
                return cache_size_amd(level);

            return 0;
        }
    }
}
