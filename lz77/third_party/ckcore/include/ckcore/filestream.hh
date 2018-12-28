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
 * @file include/ckcore/filestream.hh
 * @brief Implementation of stream interfaces for dealing with files.
 */

#pragma once
#include "ckcore/types.hh"
#include "ckcore/stream.hh"
#include "ckcore/file.hh"
#include "ckcore/path.hh"

namespace ckcore
{
    /**
     * @brief Stream class for reading files.
     */
    class FileInStream : public InStream
    {
    private:
        File file_;
        tint64 size_;
        tint64 read_;

    public:
        /**
         * Constructs a FileInStream object.
         */
        FileInStream(const Path &file_path);

        /**
         * Closes the stream and destructs the object.
         */
        virtual ~FileInStream();

        /**
         * Opens the file for access through the stream.
         * @return If successfull true is returned, otherwise false.
         */
        bool open();

        /**
         * Closes the currently opened file handle. If the file has not been opened
         * a call this call will fail.
         * @return If successfull true is returned, otherwise false.
         */
        bool close();

        /**
         * Checks if the end of the stream has been reached.
         * @return If positioned at end of the stream true is returned,
         *         otherwise false is returned.
         */
        bool end();

        /**
         * Repositions the file pointer to the specified offset accoding to the
         * whence directive in the file.
         * @param [in] distance The number of bytes that the file pointer should
         *                      move.
         * @param [in] whence Specifies what to use as base when calculating the
         *                    final file pointer position.
         * @return If successfull true is returned, otherwise false is returned.
         */
        bool seek(tuint32 distance,StreamWhence whence);

        /**
         * Checks whether the file stream has been opened or not.
         * @return If a file stream is open true is returned, otherwise false is
         *         returned.
         */
        bool test() const;

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
         * Returns the size of the file provoding data for the stream.
         * @return If successfull the size in bytes of the file is returned,
         *         if unsuccessfull -1 is returned.
         */
        tint64 size();
    };

    /**
     * @brief Stream class for writing files.
     */
    class FileOutStream : public OutStream
    {
    private:
        File file_;

    public:
        /**
         * Constructs a FileOutStream object.
         */
        FileOutStream(const Path &file_path);

        /**
         * Closes the stream and destructs the object.
         */
        virtual ~FileOutStream();

        /**
         * Opens the file for access through the stream.
         * @return If successfull true is returned, otherwise false.
         */
        bool open();

        /**
         * Closes the currently opened file handle. If the file has not been opened
         * a call this call will fail.
         * @return If successfull true is returned, otherwise false.
         */
        bool close();

        /**
         * Writes raw data to the stream.
         * @param [in] buffer Pointer to the beginning of the bufferi
         *                    containing the data to be written.
         * @param [in] count The number of bytes to write.
         * @return If the operation failed -1 is returned, otherwise the
         *         function returns the number of bytes written (this may be
         *         zero).
         */
        tint64 write(const void *buffer,tuint32 count);
    };
}
