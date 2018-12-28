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

/**
 * @file include/ckcore/memorystream.hh
 * @brief In-memory implementation of stream interfaces.
 */

#pragma once
#include "ckcore/types.hh"
#include "ckcore/stream.hh"

namespace ckcore
{
    /**
     * @brief In-memory stream class for reading streams.
     */
    class MemoryInStream : public InStream
    {
    private:
        unsigned char *data_;
        tuint32 count_;
        tuint32 pos_;

    public:
        /**
         * Constructs an MemoryInStream object. The stream doesn't take ownership of the specified
         * input data, nor does it copy it. It assumes the data is available through its life time.
         * @param [in] data Pointer to data.
         * @param [in] count Number of elements available at data pointer.
         */
        MemoryInStream(unsigned  char *data,tuint32 count);

        /**
         * Destructs the MemoryInStream object.
         */
        virtual ~MemoryInStream();

        /**
         * Checks if the end of the stream has been reached.
         * @return If positioned at end of the stream true is returned,
         *         otherwise false is returned.
         */
        bool end();

        /**
         * Repositions the file pointer to the specified offset accoding to the
         * whence directive in the stream. Please note that the seeking performance
         * is very poor since it calls the read function and throws the data.
         * @param [in] distance The number of bytes that the stream pointer should
         *                      move.
         * @param [in] whence Specifies what to use as base when calculating the
         *                    final stream pointer position.
         * @return If successfull true is returned, otherwise false is returned.
         */
        bool seek(tuint32 distance,StreamWhence whence);

        /**
         * Reads raw data from the stream.
         * @param [in] buffer Pointer to beginning of buffer to read to.
         * @param [in] count The number of bytes to read.
         * @return If the operation failed -1 is returned, otherwise the
         *         function returns the number of butes read (this may be zero
         *         when the end of the file has been reached).
         */
        tint64 read(void *buffer,tuint32 count);

        /**
         * Calculates the size of the data provided by the stream.
         * @return If successfull the size in bytes of the stream data is returned,
         *         if unsuccessfull -1 is returned.
         */
        tint64 size();
    };

    /**
     * @brief In-memory stream class for reading streams.
     */
    class MemoryOutStream : public OutStream
    {
    private:
        unsigned char *buffer_;
        tuint32 buffer_size_;
        tuint32 buffer_pos_;

    public:
        /**
         * Constructs an MemoryOutStream object.
         */
        MemoryOutStream();

        /**
         * Constructs an MemoryOutStream object.
         * @param [in] buffer_size The size of the internal buffer.
         */
        MemoryOutStream(tuint32 buffer_size);

        /**
         * Destructs the MemoryOutStream object and flushes any remaining data in
         * the buffer.
         */
        ~MemoryOutStream();

        /**
         * Writes raw data to the stream.
         * @param [in] buffer Pointer to the beginning of the bufferi
         *                    containing the data to be written.
         * @param [in] count The number of bytes to write.
         * @return If the operation failed -1 is returned, otherwise the
         *         function returns the number of bytes written.
         */
        tint64 write(const void *buffer,tuint32 count);

        /**
         * Returns a pointer to the data.
         * @return Pointer to the data.
         */
        unsigned char *data() const;

        /**
         * Returns the number of elements current stored in the stream buffer.
         * @return The number of elements current stored in the stream buffer.
         */
        tuint32 count() const;
    };
}
