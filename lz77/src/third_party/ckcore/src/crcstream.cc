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
 * aint with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <assert.h>
#include "ckcore/crcstream.hh"

namespace ckcore
{
    tuint32 CrcStream::reflect(tuint32 crc,unsigned char length)
    {
        tuint32 result = 0;

        for (tuint32 i = (tuint32)1 << (length - 1),j = 1; i; i >>= 1)
        {
            if (crc & i)
                result |= j;

            j<<= 1;
        }

        return result;
    }

    CrcStream::CrcStream(CrcType type) : reflect_(true),order_(32),
        mask_(0xffffffff),initial_(0xffffffff),
        final_(0xffffffff),checksum_(0xffffffff)
    {
        // Calculate the table entries.
        tuint32 crc = 0;
        tuint32 poly = 0x04c11db7;  // CRC-32-IEEE 802.3. 

        // Initialize depending on which type of CRC algorithm to use.
        switch (type)
        {
            case ckCRC_16:
                poly = 0x8005;      // CRC-16-IBM.
                order_ = 16;
                initial_ = 0xffff;
                final_ = 0xffff;
                checksum_ = 0xffff;
                break;

            case ckCRC_32:
                // Do nothing, this is the default configuration.
                break;

            case ckCRC_CCITT:
                poly = 0x1021;      // From UDF 1.50 reference documentation.
                reflect_ = false;
                order_ = 16;
                initial_ = 0x0000;
                final_ = 0x0000;
                checksum_ = 0x0000;
                break;

            default:
                assert(false);
        }

        tuint32 high = (tuint32)1 << (order_ - 1);
        mask_ = ((high - 1) << 1) | 1;

        for (int i = 0; i < 256; i++)
        {
            crc = (reflect_ ? reflect(i,8) : i) << (order_ - 8);

            for (int j = 0; j < 8; j++)
            {
                if (crc & high)
                    crc = (crc << 1) ^ poly;
                else
                    crc = (crc << 1);
            }

            table_[i] = (reflect_ ? reflect(crc,order_) : crc) & mask_;
        }
    }

    void CrcStream::reset()
    {
        checksum_ = initial_;
    }

    tuint32 CrcStream::checksum()
    {
        return (checksum_ ^ final_);
    }

    tint64 CrcStream::write(const void *buffer,tuint32 count)
    {
        for (unsigned long i = 0; i < count; i++)
        {
            if (reflect_)
            {
                checksum_ = (checksum_ >> 8) ^ table_[(checksum_ & 0xff) ^
                            ((const unsigned char *)buffer)[i]];

            }
            else
            {
                checksum_ = ((checksum_ << 8) ^ table_[((checksum_ >> (order_ - 8)) & 0xff) ^
                            ((const unsigned char *)buffer)[i]]) & mask_;
            }
        }

        return count;
    }
}
