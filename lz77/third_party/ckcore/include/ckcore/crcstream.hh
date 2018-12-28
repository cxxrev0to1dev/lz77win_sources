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
 * @file include/ckcore/crcstream.hh
 * @brief Stream class for calculating CRC checksums.
 */

#pragma once
#include "ckcore/types.hh"
#include "ckcore/stream.hh"

namespace ckcore
{
    /**
     * @brief Stream for calculating CRC checksums.
     */
    class CrcStream : public OutStream
    {
    public:
        /**
         * Defines different types of CRC algorithms.
         */
        enum CrcType
        {
            /**
             * Uses IBM polynomial x^16 + x^15 + x^2 + 1.
             */
            ckCRC_16,

            /**
             * Uses IEEE 802.3 polynomial x^32 + x^26 + x^23 + x^22 + x^16 +
             * x^12 + x^11 + x^10 + x^8 + x^7 + x^5 + x^4 + x^2 + x + 1.
             * */
            ckCRC_32,

            /**
             * Uses CCITT polynomial x^16 + x^12 + x^5 + 1.
             */
            ckCRC_CCITT
        };

    private:
        bool reflect_;
        unsigned char order_;   // Which order of CRC (8,16,32,...).
        tuint32 mask_;          // Mask of all bits in the checksum.
        tuint32 initial_;       // Initial checksum (for reset function).
        tuint32 final_;         // Value to xor with final checksum.
        tuint32 checksum_;      // Current checksum.
        tuint32 table_[256];

        tuint32 reflect(tuint32 crc,unsigned char length);

    public:
        /**
         * Constructs a CrcStream object.
         */
        CrcStream(CrcType type);

        /**
         * Resets the internal CRC checksum.
         */
        void reset();

        /**
         * Returns the internal checksum.
         * @return The internal checksum.
         */
        tuint32 checksum();

        /**
         * Updates the internal checksum according to the data in the specified
         * buffer.
         * @param [in] buffer Pointer to the beginning of a buffer containing the
         *                    data to calculate the checksum of.
         * @param [in] count The number of bytes in the buffer.
         * @return The number of bytes processed (always the same as count).
         */
        tint64 write(const void *buffer,tuint32 count);
    };
}
