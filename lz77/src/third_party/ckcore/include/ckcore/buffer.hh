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
 * @file include/ckcore/buffer.hh
 * @brief Buffer class with facilities for resizing and deallocation.
 */

#pragma once
#include "ckcore/types.hh"

namespace ckcore
{
    /**
     * @brief Buffer class with facilities for resizing and deallocation.
     */
    template <typename T,typename S = size_t>
    class Buffer
    {
    private:
        /**
         * @brief Defines buffer class properties.
         */
        enum
        {
            DEFAULT_BUFFER_SIZE = 4096
        };

    private:
        S size_;
        T *buffer_;

    public:
        /**
         * Constructs the Buffer object.
         */
        Buffer()
            : size_(DEFAULT_BUFFER_SIZE),
              buffer_(new T [DEFAULT_BUFFER_SIZE])
        {
        }

        /**
         * Constructs the Buffer object.
         * @param [in] size Buffer size.
         */
        Buffer(S size)
            : size_(size),
              buffer_(new T [size])
        {
        }

        /**
         * Destructs the Buffer object.
         */
        ~Buffer()
        {
            if (buffer_ != NULL)
            {
                delete [] buffer_;
                buffer_ = NULL;
            }
        }

        /**
         * Resizes the buffer, previous buffer contents will be lost.
         * @param [in] size Buffer size.
         */
        void resize(S size)
        {
            if (buffer_ != NULL)
                delete [] buffer_;

            size_ = size;
            buffer_ = new T [size_];
        }

        /**
         * Returns the buffer size.
         * @return The buffer size.
         */
        S size() const
        {
            return size_;
        }

        /**
         * Type conversion operator returning the buffer pointer.
         * @return Buffer pointer.
         */
        operator T *()
        {
            return buffer_;
        }
    };
}
