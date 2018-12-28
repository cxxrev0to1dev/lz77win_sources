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

#include "ckcore/filestream.hh"

#include <assert.h>

namespace ckcore
{
    FileInStream::FileInStream(const Path &file_path)
      : file_(file_path)
      , read_(0)
    {
      // TODO: we should make all callers exception safe, because
      //       it's hard to be certain that everybody always checks
      //       whether the size_ is -1 before trying to use
      //       this object.
      try
      {
        size_ = file_.size2();
      }
      catch ( ... )
      {
        size_ = -1;
      }
    }

    FileInStream::~FileInStream()
    {
        close();
    }

    bool FileInStream::open()
    {
        try
        {
          size_ = file_.size2();
        }
        catch ( ... )
        {
          size_ = -1;
        }
        
        try
        {
          file_.open2(File::ckOPEN_READ);
          return true;
        }
        catch ( ... )
        {
          return false;
        }
    }

    bool FileInStream::close()
    {
        if (file_.close())
        {
            read_ = 0;
            return true;
        }

        return false;
    }

    bool FileInStream::end()
    {
        return read_ >= size_;
    }

    bool FileInStream::seek(tuint32 distance,StreamWhence whence)
    {
        File::FileWhence file_whence;
        switch (whence)
        {
            case ckSTREAM_CURRENT:
                file_whence = File::ckFILE_CURRENT;
                break;

            default:
                file_whence = File::ckFILE_BEGIN;
                break;
        }

        try
        {
            tint64 result = file_.seek2(distance,file_whence);
            assert( result != -1 );  // Errors throw now exceptions.
            read_ = result;
            return true;
        }
        catch ( ... )
        {
          return false;
        }
    }

    bool FileInStream::test() const
    {
        return file_.test();
    }

    tint64 FileInStream::read(void *buffer,tuint32 count)
    {
        tint64 result = file_.read(buffer,count);
        if (result != -1)
            read_ += result;

        return result;
    }

    tint64 FileInStream::size()
    {
        return size_;
    }

    FileOutStream::FileOutStream(const Path &file_path) : file_(file_path)
    {
    }

    FileOutStream::~FileOutStream()
    {
        close();
    }

    bool FileOutStream::open()
    {
      try
      {
        file_.open2(File::ckOPEN_WRITE);
        return true;
      }
      catch ( ... )
      {
        return false;
      }
    }

    bool FileOutStream::close()
    {
        return file_.close();
    }

    tint64 FileOutStream::write(const void *buffer,tuint32 count)
    {
        return file_.write(buffer,count);
    }
}
