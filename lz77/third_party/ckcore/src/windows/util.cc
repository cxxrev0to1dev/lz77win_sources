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

#include "stdafx.hh"
#include "util.hh"

namespace ckcore
{
    void SysTimeToTm(SYSTEMTIME &stime,struct tm &time)
    {
        time.tm_sec = stime.wSecond;
        time.tm_min = stime.wMinute;
        time.tm_hour = stime.wHour;
        time.tm_mday = stime.wDay;
        time.tm_mon = stime.wMonth - 1;
        time.tm_year = stime.wYear - 1900;
        time.tm_wday = stime.wDayOfWeek;

        // Caclulate the day of the year.
        int days = 0;

        if ((time.tm_year % 400 == 0) ||
            ((time.tm_year % 4 == 0) && (time.tm_year % 100 != 0)))
        {
            unsigned char month_len[] = { 31,29,31,30,31,30,31,31,30,31,30,31 };

            for (int i = 0; i < time.tm_mon; i++)
                days += month_len[0];
        }
        else
        {
            unsigned char month_len[] = { 31,28,31,30,31,30,31,31,30,31,30,31 };

            for (int i = 0; i < time.tm_mon; i++)
                days += month_len[0];
        }

        time.tm_yday = days + time.tm_mday;

        // Get day light saving attribute.
        TIME_ZONE_INFORMATION tzi;
        time.tm_isdst = GetTimeZoneInformation(&tzi) - 1;
    }
};
