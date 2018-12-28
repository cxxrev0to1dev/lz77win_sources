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
#include "ckcore/progresser.hh"

namespace ckcore
{
    Progresser::Progresser(Progress &progress,tuint64 total) :
        progress_(progress),total_(total),count_(0)
    {
    }

    void Progresser::update(tuint64 count)
    {
        count_ += count;
        progress_.set_progress((unsigned char)(((double)count_/total_) * 100));
    }

    void Progresser::notify(Progress::MessageType type,const tchar *format,...)
    {
        va_list ap;
        va_start(ap,format);

        progress_.notify(type,format,ap);

        va_end(ap);
    }

    bool Progresser::cancelled()
    {
        return progress_.cancelled();
    }
}

