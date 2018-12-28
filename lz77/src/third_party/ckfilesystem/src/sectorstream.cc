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

#include "ckfilesystem/sectorstream.hh"

namespace ckfilesystem
{
    /*
        SectorOutStream
    */
    SectorInStream::SectorInStream(ckcore::InStream &in_stream,
                                   ckcore::tuint32 sector_size) :
        ckcore::CanexInStream(in_stream,ckT("")),sector_size_(sector_size),
        sector_(0),read_(0)
    {
    }

    SectorInStream::~SectorInStream()
    {
    }

    void SectorInStream::seek(ckcore::tuint32 distance,ckcore::InStream::StreamWhence whence)
    {
        ckcore::CanexInStream::seek(distance,whence);

        switch (whence)
        {
        case ckcore::InStream::ckSTREAM_BEGIN:
            sector_ = 0;
            read_ = distance;
            break;

        case ckcore::InStream::ckSTREAM_CURRENT:
            read_ += distance;          
            break;
        }

        // Advance the sector counter.
        while (read_ >= sector_size_)
        {
            read_ -= sector_size_;
            sector_++;
        }
    }

    ckcore::tint64 SectorInStream::read(void *buffer,ckcore::tuint32 count)
    {
        ckcore::tint64 res = ckcore::CanexInStream::read(buffer,count);

        read_ += count;

        while (read_ >= sector_size_)
        {
            read_ -= sector_size_;
            sector_++;
        }

        return res;
    }

    /*
        Returns the current sector number.
    */
    ckcore::tuint64 SectorInStream::get_sector()
    {
        return sector_;
    }

    /*
        Returns the remaining unallocated bytes in the current sector.
    */
    ckcore::tuint32 SectorInStream::get_remaining()
    {
        return sector_size_ - (ckcore::tuint32)read_;
    }

    /*
        SectorOutStream
    */
    SectorOutStream::SectorOutStream(ckcore::OutStream &out_stream,
                                     ckcore::tuint32 sector_size) :
        ckcore::CanexOutStream(out_stream,ckT("")),sector_size_(sector_size),
        sector_(0),written_(0)
    {
    }

    SectorOutStream::~SectorOutStream()
    {
    }

    void SectorOutStream::write(void *buffer,ckcore::tuint32 count)
    {
        ckcore::CanexOutStream::write(buffer,count);
        written_ += count;

        while (written_ >= sector_size_)
        {
            written_ -= sector_size_;
            sector_++;
        }
    }

    /*
        Returns the current sector number.
    */
    ckcore::tuint64 SectorOutStream::get_sector()
    {
        return sector_;
    }

    /*
        Returns the number of buytes that's allocated in the current sector.
    */
    ckcore::tuint32 SectorOutStream::get_allocated()
    {
        return (ckcore::tuint32)written_;
    }

    /*
        Returns the remaining unallocated bytes in the current sector.
    */
    ckcore::tuint32 SectorOutStream::get_remaining()
    {
        return sector_size_ - (ckcore::tuint32)written_;
    }

    /*
        Pads the remaining bytes of the current sector with 0s.
    */
    void SectorOutStream::pad_sector()
    {
        char tmp[1] = { 0 };

        ckcore::tuint32 remaining = get_remaining();
        for (ckcore::tuint32 i = 0; i < remaining; i++)
            write(tmp,1);
    }
};
