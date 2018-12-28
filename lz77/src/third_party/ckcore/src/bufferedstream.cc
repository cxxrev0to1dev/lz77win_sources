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
#include "ckcore/system.hh"
#include "ckcore/bufferedstream.hh"

namespace ckcore
{
    BufferedInStream::BufferedInStream(InStream &stream) : stream_(stream),
        buffer_(NULL),buffer_size_(0),buffer_pos_(0),buffer_data_(0)
    {
        // UPDATE: Hangs the application on some systems.
        /*buffer_size_ = System::Cache(System::ckLEVEL_1);
        if (buffer_size_ == 0)*/
            buffer_size_ = 8192;

        buffer_ = new unsigned char[buffer_size_];

        // Make sure that the memory allocation succeeded.
        if (buffer_ == NULL)
            buffer_size_ = 0;
    }

    BufferedInStream::BufferedInStream(InStream &stream,
                                       tuint32 buffer_size) :
        stream_(stream),buffer_(NULL),buffer_size_(buffer_size),buffer_pos_(0),
        buffer_data_(0)
    {
        if (buffer_size_ == 0)
            buffer_size_ = 8192;

        buffer_ = new unsigned char[buffer_size_];

        // Make sure that the memory allocation succeeded.
        if (buffer_ == NULL)
            buffer_size_ = 0;
    }

    BufferedInStream::~BufferedInStream()
    {
        // Free the memory allocated for the internal buffer.
        if (buffer_ != NULL)
        {
            delete [] buffer_;
            buffer_ = NULL;
        }
    }

    bool BufferedInStream::end()
    {
        return stream_.end() && buffer_data_ == 0;
    }

    bool BufferedInStream::seek(tuint32 distance,StreamWhence whence)
    {
        // Reset the internal state if necessary.
        if (whence == ckSTREAM_BEGIN)
        {
            if (!stream_.seek(0,ckSTREAM_BEGIN))
                return false;

            buffer_pos_ = 0;
            buffer_data_ = 0;
        }

        // Optimization.
        if (distance == 0)
            return true;

        // Perform the seek operation.
        tuint32 buffer_size = buffer_size_ == 0 ? 8192 : buffer_size_;
        unsigned char *temp_buffer = new unsigned char[buffer_size];
        while (distance > 0)
        {
            tuint32 read_bytes = distance > buffer_size ? buffer_size : distance;

            tint64 res = read(temp_buffer,read_bytes);
            if (res == -1)
            {
                delete [] temp_buffer;
                return false;
            }

            distance -= (tuint32)res;
        }

        delete [] temp_buffer;
        return true;
    }

    tint64 BufferedInStream::read(void *buffer,tuint32 count)
    {
        // If we have failed to allocate the internal buffer, just redirect the
        // read call.
        if (buffer_size_ == 0)
            return stream_.read(buffer,count);

        tuint32 pos = 0;

        while (count > buffer_data_)
        {
            memcpy((unsigned char *)buffer + pos,buffer_ + buffer_pos_,buffer_data_);
            count -= buffer_data_;

            pos += buffer_data_;

            buffer_pos_ = 0;
            buffer_data_ = 0;

            // Fetch more data from the input stream.
            if (stream_.end())
                return pos;

            tint64 result = stream_.read(buffer_,buffer_size_);
            if (result == -1)
                return pos == 0 ? -1 : pos;

            buffer_data_ = (tuint32)result;
        }

        memcpy((unsigned char *)buffer + pos,buffer_ + buffer_pos_,count);
        buffer_pos_ += count;
        buffer_data_ -= count;

        return pos + count;
    }

    tint64 BufferedInStream::size()
    {
        return stream_.size();
    }

    BufferedOutStream::BufferedOutStream(OutStream &stream) : stream_(stream),
        buffer_(NULL),buffer_size_(0),buffer_pos_(0)
    {
        // UPDATE: Hangs the application on some systems.
        /*buffer_size_ = System::Cache(System::ckLEVEL_1);
        if (buffer_size_ == 0)*/
            buffer_size_ = 8192;

        buffer_ = new unsigned char[buffer_size_];

        // Make sure that the memory allocation succeeded.
        if (buffer_ == NULL)
            buffer_size_ = 0;
    }

    BufferedOutStream::BufferedOutStream(OutStream &stream,
                                         tuint32 buffer_size) :
        stream_(stream),buffer_(NULL),buffer_size_(buffer_size),buffer_pos_(0)
    {
        if (buffer_size_ == 0)
            buffer_size_ = 8192;

        buffer_ = new unsigned char[buffer_size_];

        // Make sure that the memory allocation succeeded.
        if (buffer_ == NULL)
            buffer_size_ = 0;
    }

    BufferedOutStream::~BufferedOutStream()
    {
        flush();

        // Free the memory allocated for the internal buffer.
        if (buffer_ != NULL)
        {
            delete [] buffer_;
            buffer_ = NULL;
        }
    }

    tint64 BufferedOutStream::write(const void *buffer,tuint32 count)
    {
        // If we failed to allocate the internal buffer, just redirect the
        // write call.
        if (buffer_size_ == 0)
            return stream_.write(buffer,count);

        tuint32 pos = 0;

        while (buffer_pos_ + count > buffer_size_)
        {
            tuint32 remain = buffer_size_ - buffer_pos_;
            memcpy(buffer_ + buffer_pos_,(unsigned char *)buffer + pos,remain);

            pos += remain;

            // Flush.
            if (stream_.write(buffer_,buffer_size_) == -1)
                return pos == 0 ? -1 : pos;

            buffer_pos_ = 0;

            count -= remain;
        }

        memcpy(buffer_ + buffer_pos_,(unsigned char *)buffer + pos,count);
        buffer_pos_ += count;

        return pos + count;
    }

    tint64 BufferedOutStream::flush()
    {
        // If we don't have a buffer we can't flush.
        if (buffer_size_ == 0)
            return 0;

        tint64 result = stream_.write(buffer_,buffer_pos_);
        if (result != -1)
            buffer_pos_ = 0;

        return result;
    }
}
