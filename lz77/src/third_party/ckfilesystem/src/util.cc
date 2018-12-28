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

#include "ckfilesystem/iso.hh"

namespace ckfilesystem
{
    namespace util
    {
        /**
         * Caluclates the number of sectors needed to stored a certain number
         * of bytes.
         * @param [in] bytes The number of bytes to store.
         * @return The number of sectors needed to store the bytes.
         */
        ckcore::tuint32 bytes_to_sec(ckcore::tuint32 bytes)
        {
            if (bytes == 0)
                return 0;

            ckcore::tuint32 sectors = 1;
            while (bytes > ISO_SECTOR_SIZE)
            {
                bytes -= ISO_SECTOR_SIZE;
                sectors++;
            }

            return sectors;
        }

        /**
         * Caluclates the number of sectors needed to stored a certain number
         * of bytes.
         * @param [in] bytes The number of bytes to store.
         * @return The number of sectors needed to store the bytes.
         */
        ckcore::tuint32 bytes_to_sec(ckcore::tuint64 bytes)
        {
            if (bytes == 0)
                return 0;

            ckcore::tuint32 sectors = 1;
            while (bytes > ISO_SECTOR_SIZE)
            {
                bytes -= ISO_SECTOR_SIZE;
                sectors++;
            }

            return sectors;
        }

        /**
         * Caluclates the number of sectors needed to stored a certain number
         * of bytes.
         * @param [in] bytes The number of bytes to store.
         * @return The number of sectors needed to store the bytes.
         */
        ckcore::tuint64 bytes_to_sec64(ckcore::tuint64 bytes)
        {
            if (bytes == 0)
                return 0;

            ckcore::tuint64 sectors = 1;
            while (bytes > ISO_SECTOR_SIZE)
            {
                bytes -= ISO_SECTOR_SIZE;
                sectors++;
            }

            return sectors;
        }

        /**
         * Writes a 16-bit integer to the specified buffer in least significant
         * byte first order. This is in accordance to ECMA-119: 7.2.1.
         * @param [out] buffer The buffer to write the value to. It must be at
         *                     least 2 bytes large.
         * @param [in] val The integer value to write to the buffer.
         */
        void write721(unsigned char *buffer,ckcore::tuint16 val)
        {
            buffer[0] = val & 0xff;
            buffer[1] = (val >> 8) & 0xff;
        }

        /**
         * Writes a 16-bit integer to the specified buffer in most significant
         * byte first order. This is in accordance to ECMA-119: 7.2.2.
         * @param [out] buffer The buffer to write the value to. It must be at
         *                     least 2 bytes large.
         * @param [in] val The integer value to write to the buffer.
         */
        void write722(unsigned char *buffer,ckcore::tuint16 val)
        {
            buffer[0] = (val >> 8) & 0xff;
            buffer[1] = val & 0xff;
        }

        /**
         * Writes a 16-bit integer to the specified buffer in both least
         * significant first and most significant byte first order. This is in
         * accordance to ECMA-119: 7.2.3.
         * @param [out] buffer The buffer to write the value to. It must be at
         *                     least 4 bytes large.
         * @param [in] val The integer value to write to the buffer.
         */
        void write723(unsigned char *buffer,ckcore::tuint16 val)
        {
            buffer[3] = buffer[0] = val & 0xff;
            buffer[2] = buffer[1] = (val >> 8) & 0xff;
        }

        /**
         * Writes a 32-bit integer to the specified buffer in least significant
         * byte first order. This is in accordance to ECMA-119: 7.3.1.
         * @param [out] buffer The buffer to write the value to. It must be
         *                     least 4 bytes large.
         * @param [in] val The integer value to write to the buffer.
         */
        void write731(unsigned char *buffer,ckcore::tuint32 val)
        {
            buffer[0] = (unsigned char)(val & 0xff);
            buffer[1] = (unsigned char)((val >> 8) & 0xff);
            buffer[2] = (unsigned char)((val >> 16) & 0xff);
            buffer[3] = (unsigned char)((val >> 24) & 0xff);
        }

        /**
         * Writes a 32-bit integer to the specified buffer in most significant
         * byte first order. This is in accordance to ECMA-119: 7.3.2.
         * @param [out] buffer The buffer to write the value to. It must be at
         *                     least 4 bytes large.
         * @param [in] val The integer value to write to the buffer.
         */
        void write732(unsigned char *buffer,ckcore::tuint32 val)
        {
            buffer[0] = (unsigned char)((val >> 24) & 0xff);
            buffer[1] = (unsigned char)((val >> 16) & 0xff);
            buffer[2] = (unsigned char)((val >> 8) & 0xff);
            buffer[3] = (unsigned char)(val & 0xff);
        }

        /**
         * Writes a 32-bit integer to the specified buffer in both least
         * significant first and most significant byte first order. This is in
         * accordance to ECMA-119: 7.3.3.
         * @param [out] buffer The buffer to write the value to. It must be at
         *                     least 8 bytes large.
         * @param [in] val The integer value to write to the buffer.
         */
        void write733(unsigned char *buffer,ckcore::tuint32 val)
        {
            buffer[7] = buffer[0] = (unsigned char)(val & 0xff);
            buffer[6] = buffer[1] = (unsigned char)((val >> 8) & 0xff);
            buffer[5] = buffer[2] = (unsigned char)((val >> 16) & 0xff);
            buffer[4] = buffer[3] = (unsigned char)((val >> 24) & 0xff);
        }

        /**
         * Reads a 16-bit integer from the specified buffer in least
         * significant byte first order. This is in accordance to
         * ECMA-119: 7.2.1.
         * @param [in] buffer The buffer to read the value from. It must be at
         *                    least 2 bytes large.
         * @return The integer value read from the buffer.
         */
        ckcore::tuint16 read721(const unsigned char *buffer)
        {
            return ((ckcore::tuint16)buffer[1] << 8) | buffer[0];
        }

        /**
         * Reads a 16-bit integer from the specified buffer in most significant
         * byte first order. This is in accordance to ECMA-119: 7.2.2.
         * @param [in] buffer The buffer to read the value from. It must be at
         *                    least 2 bytes large.
         * @return The integer value read from the buffer.
         */
        ckcore::tuint16 read722(const unsigned char *buffer)
        {
            return ((ckcore::tuint16)buffer[0] << 8) | buffer[1];
        }

        /**
         * Reads a 16-bit integer from the specified buffer in both least
         * significant first and most significant byte first order. This is in
         * accordance to ECMA-119: 7.2.3.
         * @param [in] buffer The buffer to read the value from. It must be at
         *                    least 2 bytes large.
         * @return The integer value read from the buffer.
         */
        ckcore::tuint16 read723(const unsigned char *buffer)
        {
            return read721(buffer);
        }

        /**
         * Reads a 32-bit integer from the specified buffer in least
         * significant byte first order. This is in accordance to
         * ECMA-119: 7.3.1.
         * @param [in] buffer The buffer to read the value from. It must be at
         *                    least 2 bytes large.
         * @return The integer value read from the buffer.
         */
        ckcore::tuint32 read731(const unsigned char *buffer)
        {
            return ((ckcore::tuint32)buffer[3] << 24) | ((ckcore::tuint32)buffer[2] << 16) |
                ((ckcore::tuint32)buffer[1] << 8) | buffer[0];
        }

        /**
         * Reads a 32-bit integer from the specified buffer in most significant
         * byte first order. This is in accordance to ECMA-119: 7.3.2.
         * @param [in] buffer The buffer to read the value from. It must be at
         *                    least 2 bytes large.
         * @return The integer value read from the buffer.
         */
        ckcore::tuint32 read732(const unsigned char *buffer)
        {
            return ((ckcore::tuint32)buffer[0] << 24) | ((ckcore::tuint32)buffer[1] << 16) |
                ((ckcore::tuint32)buffer[2] << 8) | buffer[3];
        }

        /**
         * Reads a 32-bit integer from the specified buffer in both least
         * significant first and most significant byte first order. This is in
         * accordance to ECMA-119: 7.3.3.
         * @param [in] buffer The buffer to read the value from. It must be at
         *                    least 2 bytes large.
         * @return The integer value read from the buffer.
         */
        ckcore::tuint32 read733(const unsigned char *buffer)
        {
            return read731(buffer);
        }
    }
};

