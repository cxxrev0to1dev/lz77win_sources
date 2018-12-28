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
#include "ckcore/assert.hh"
#include "ckcore/memorystream.hh"

namespace ckcore
{
    MemoryInStream::MemoryInStream(unsigned char * data,tuint32 count) :
        data_(data),count_(count),pos_(0)
    {
        ckASSERT(data);
    }

    MemoryInStream::~MemoryInStream()
    {
    }

    bool MemoryInStream::end()
    {
        return pos_ >= count_;
    }

    bool MemoryInStream::seek(tuint32 distance,StreamWhence whence)
    {
        // Reset the internal state if necessary.
        if (whence == ckSTREAM_BEGIN)
            pos_ = 0;

        pos_ += distance;
        return true;
    }

    tint64 MemoryInStream::read(void *buffer,tuint32 count)
    {
        tuint32 to_read = pos_ + count > count_ ? count_ - pos_ : count;
        memcpy(buffer,data_ + pos_,to_read);
        pos_ += to_read;

        return to_read;
    }

    tint64 MemoryInStream::size()
    {
        return count_;
    }

    MemoryOutStream::MemoryOutStream() : 
        buffer_(NULL),buffer_size_(1024),buffer_pos_(0)
    {
        buffer_ = new unsigned char[buffer_size_];

        // Make sure that the memory allocation succeeded.
        if (buffer_ == NULL)
            buffer_size_ = 0;
    }

    MemoryOutStream::MemoryOutStream(tuint32 buffer_size) :
        buffer_(NULL),buffer_size_(buffer_size),buffer_pos_(0)
    {
        if (buffer_size_ == 0)
            buffer_size_ = 1024;

        buffer_ = new unsigned char[buffer_size_];

        // Make sure that the memory allocation succeeded.
        if (buffer_ == NULL)
            buffer_size_ = 0;
    }

    MemoryOutStream::~MemoryOutStream()
    {
        // Free the memory allocated for the internal buffer.
        if (buffer_ != NULL)
        {
            delete [] buffer_;
            buffer_ = NULL;
        }
    }

    tint64 MemoryOutStream::write(const void *buffer,tuint32 count)
    {
        // Make sure we have a buffer to write to.
        if (buffer_ == NULL)
            return -1;

        while (buffer_pos_ + count > buffer_size_)
        {
            tuint32 new_buffer_size = buffer_size_ * 2;
            unsigned char *new_buffer = new unsigned char[new_buffer_size];
            if (new_buffer == NULL)
                return -1;

            memcpy(new_buffer,buffer_,buffer_size_);
            delete [] buffer_;

            buffer_ = new_buffer;
            buffer_size_ = new_buffer_size;
        }

        memcpy(buffer_ + buffer_pos_,buffer,count);
        buffer_pos_ += count;
        return count;
    }

    unsigned char *MemoryOutStream::data() const
    {
        return buffer_;
    }

    tuint32 MemoryOutStream::count() const
    {
        return buffer_pos_;
    }
}
