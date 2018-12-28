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
#include <ckcore/bufferedstream.hh>
#include <ckcore/canexstream.hh>
#include "ckfilesystem/iso.hh"

namespace ckfilesystem
{
    class SectorInStream : public ckcore::CanexInStream
    {
    private:
        ckcore::tuint32 sector_size_;
        ckcore::tuint64 sector_;
        ckcore::tuint64 read_;

    public:
        SectorInStream(ckcore::InStream &in_stream,
                       ckcore::tuint32 sector_size = ISO_SECTOR_SIZE);
        virtual ~SectorInStream();

        void seek(ckcore::tuint32 distance,ckcore::InStream::StreamWhence whence);
        ckcore::tint64 read(void *buffer,ckcore::tuint32 count);

        ckcore::tuint64 get_sector();
        ckcore::tuint32 get_remaining();
    };

    class SectorOutStream : public ckcore::CanexOutStream
    {
    private:
        ckcore::tuint32 sector_size_;
        ckcore::tuint64 sector_;
        ckcore::tuint64 written_;

    public:
        SectorOutStream(ckcore::OutStream &out_stream,
                        ckcore::tuint32 sector_size = ISO_SECTOR_SIZE);
        virtual ~SectorOutStream();

        void write(void *buffer,ckcore::tuint32 count);

        ckcore::tuint64 get_sector();
        ckcore::tuint32 get_allocated();
        ckcore::tuint32 get_remaining();

        void pad_sector();
    };
};
