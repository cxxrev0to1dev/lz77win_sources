/*
 * The ckFileSystem library provides file system functionality.
 * Copyright (C) 2006-2011 Christian Kindahl
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

#pragma once
#include <ckcore/types.hh>
#include <ckcore/stream.hh>

namespace ckfilesystem
{
    namespace util
    {
        ckcore::tuint32 bytes_to_sec(ckcore::tuint32 bytes);
        ckcore::tuint32 bytes_to_sec(ckcore::tuint64 bytes);
        ckcore::tuint64 bytes_to_sec64(ckcore::tuint64 bytes);

        void write721(unsigned char *buffer,ckcore::tuint16 val);
        void write722(unsigned char *buffer,ckcore::tuint16 val);
        void write723(unsigned char *buffer,ckcore::tuint16 val);
        void write731(unsigned char *buffer,ckcore::tuint32 val);
        void write732(unsigned char *buffer,ckcore::tuint32 val);
        void write733(unsigned char *buffer,ckcore::tuint32 val);
        ckcore::tuint16 read721(const unsigned char *buffer);
        ckcore::tuint16 read722(const unsigned char *buffer);
        ckcore::tuint16 read723(const unsigned char *buffer);
        ckcore::tuint32 read731(const unsigned char *buffer);
        ckcore::tuint32 read732(const unsigned char *buffer);
        ckcore::tuint32 read733(const unsigned char *buffer);
    };
};

