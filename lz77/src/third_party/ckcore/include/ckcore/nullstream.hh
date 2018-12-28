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
 * @file include/ckcore/nullstream.hh
 * @brief Implementation of stream interface for writing to null device.
 */

#pragma once
#include "ckcore/types.hh"
#include "ckcore/stream.hh"

namespace ckcore
{
    /**
     * @brief Stream class for writing to the null device.
     */
    class NullStream : public OutStream
    {
    private:
        tuint64 written_;

    public:
        /**
         * Constructs a NullStream object.
         */
        NullStream();

        /**
         * Returns the number of bytes written to the stream.
         * @return The number of bytes written.
         */
        tuint64 written();

        /**
         * Counts the number of bytes to be written.
         * @param [in] buffer Pointer to the beginning of the bufferi
         *                    containing the data to be written (may be NULL).
         * @param [in] count The number of bytes to write.
         * @return The function returns count.
         */
        tint64 write(const void *buffer,tuint32 count);
    };
}
