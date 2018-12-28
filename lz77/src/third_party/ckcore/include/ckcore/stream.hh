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
 * @file include/ckcore/stream.hh
 * @brief Defines the stream interfaces.
 */

#pragma once
#include "ckcore/types.hh"
#include "ckcore/progress.hh"
#include "ckcore/progresser.hh"

namespace ckcore
{
    /**
     * @brief Interface for input streams.
     */
    class InStream
    {
    public:
        virtual ~InStream() {};

        /**
         * Defines directives what to use as base offset when performing seek
         * operations.
         */
        enum StreamWhence
        {
            ckSTREAM_CURRENT,
            ckSTREAM_BEGIN
        };

        /**
         * Reads raw data from the stream.
         * @param [in] buffer Pointer to beginning of buffer to read to.
         * @param [in] count The number of bytes to read.
         * @return If the operation failed -1 is returned, otherwise the
         *         function returns the number of bytes read (this may be zero
         *         when the end of the file has been reached).
         */
        virtual tint64 read(void *buffer,tuint32 count) = 0;

        /**
         * Calculates the size of the data provided by the stream.
         * @return If successfull the size in bytes of the stream data is returned,
         *         if unsuccessfull -1 is returned.
         */
        virtual tint64 size() = 0;

        /**
         * Checks if the end of the stream has been reached.
         * @return If positioned at end of the stream true is returned,
         *         otherwise false is returned.
         */
        virtual bool end() = 0;

        /**
         * Repositions the internal stream pointer to the specified offset accoding
         * to the whence directive.
         * @param [in] distance The number of bytes that the stream pointer should
         *                      move.
         * @param [in] whence Specifies what to use as base when calculating the
         *                    final stream pointer position.
         * @return If successfull true is returned, oterwise false is returned.
         */
        virtual bool seek(tuint32 distance,StreamWhence whence) = 0;
    };

    /**
     * @brief Interface for output streams.
     */
    class OutStream
    {
    public:
        virtual ~OutStream() {};

        /**
         * Writes raw data to the stream.
         * @param [in] buffer Pointer to the beginning of the bufferi
         *                    containing the data to be written.
         * @param [in] count The number of bytes to write.
         * @return If the operation failed -1 is returned, otherwise the
         *         function returns the number of bytes written (this may be
         *         zero).
         */
        virtual tint64 write(const void *buffer,tuint32 count) = 0;
    };

    namespace stream
    {
        /**
         * Copies the contents of the input stream to the output stream. An
         * internal buffer is used to optimize the process.
         * @param [in] from The source stream.
         * @param [in] to The target stream.
         * @return If successfull true is returned, otherwise false is
         *         returned.
         */
        bool copy(InStream &from,OutStream &to);

        /**
         * Copies the contents of the input stream to the output stream. An
         * internal buffer is used to optimize the process. Progress is
         * reported through the Progress interface.
         * @param [in] from The source stream.
         * @param [in] to The target stream.
         * @param [in] progress The progress interface to report progress to.
         * @return If successfull true is returned, otherwise false is
         *         returned. Cancelling the operation is considered a failure.
         */
        bool copy(InStream &from,OutStream &to,Progress &progress);

        /**
         * Copies the contents of the input stream to the output stream. An
         * internal buffer is used to optimize the process. Progress is
         * reported through a Progresser object.
         * @param [in] from The source stream.
         * @param [in] to The target stream.
         * @param [in] progresser A reference to the progresser object to use
         *                        for reporting progress.
         * @return If successfull true is returned, otherwise false is
         *         returned. Cancelling the operation is considered a failure.
         */
        bool copy(InStream &from,OutStream &to,Progresser &progresser);

        /**
         * Copies the contents of the input stream to the output stream. An
         * internal buffer is used to optimize the process. Progress is
         * reported through a Progresser object. If the available data in the
         * input stream is less than requested the output stream will be padded
         * to match the requested ammount. If more data is available in the
         * input stream than what is requested the additional data will be ignored.
         * @param [in] from The source stream.
         * @param [in] to The target stream.
         * @param [in] progresser A reference to the progresser object to use
         *                        for reporting progress.
         * @param [in] size The exact number of bytes to write to the output
         *                  stream.
         * @return If successfull true is returned, otherwise false is
         *         returned. Cancelling the operation is considered a failure.
         */
        bool copy(InStream &from,OutStream &to,Progresser &progresser,
                  tuint64 size);
    }
}
