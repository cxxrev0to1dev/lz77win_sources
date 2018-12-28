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
 * @file include/ckcore/canexstream.hh
 * @brief Implementation of stream interfaces for dealing with files.
 */

#pragma once
#include "ckcore/types.hh"
#include "ckcore/stream.hh"

namespace ckcore
{
    /**
     * @brief Canonical input stream using exceptions.
     */
    class CanexInStream
    {
    private:
        InStream &stream_;
        tstring ident_;

    public:
        virtual ~CanexInStream() {}

        /**
         * Constructs a CanexInStream object.
         * @param [in] stream The original non-canonical input stream.
         * @param [in] ident Name for identifying the stream.
         */
        CanexInStream(InStream &stream,const tchar *ident);

        /**
         * Constructs a CanexInStream object.
         * @param [in] stream The original non-canonical input stream.
         * @param [in] ident Name for identifying the stream.
         */
        CanexInStream(InStream &stream,const tstring &ident);

        /**
         * Returns the stream identifier name.
         * @return The stream identifier name.
         */
        const tstring &identifier() const;

        /**
         * Repositions the file pointer to the specified offset accoding to the
         * whence directive in the file.
         * @param [in] distance The number of bytes that the file pointer should
         *                      move.
         * @param [in] whence Specifies what to use as base when calculating the
         *                    final file pointer position.
         * @throw Exception If seek error occurred.
         */
        virtual void seek(tuint32 distance,InStream::StreamWhence whence);

        /**
         * Reads raw data from the stream.
         * @param [in] buffer Pointer to beginning of buffer to read to.
         * @param [in] count The number of bytes to read.
         * @return The number of bytes read (this may be zero
         *         when the end of the file has been reached).
         * @throw Exception If a read error occurred.
         */
        virtual tint64 read(void *buffer,tuint32 count);

        /**
         * Checks if the end of the stream has been reached.
         * @return If positioned at end of the stream true is returned,
         *         otherwise false is returned.
         */
        bool end();
    };

    /**
     * @brief Canonical output stream using exceptions.
     */
    class CanexOutStream
    {
    private:
        OutStream &stream_;
        tstring ident_;

    public:
        virtual ~CanexOutStream() {}

        /**
         * Constructs a CanexOutStream object.
         * @param [in] stream The original non-canonical output stream.
         * @param [in] ident Name for identifying the stream.
         */
        CanexOutStream(OutStream &stream,const tchar *ident);

        /**
         * Constructs a CanexOutStream object.
         * @param [in] stream The original non-canonical output stream.
         * @param [in] ident Name for identifying the stream.
         */
        CanexOutStream(OutStream &stream,const tstring &ident);

        /**
         * Returns the stream identifier name.
         * @return The stream identifier name.
         */
        const tstring &identifier() const;

        /**
         * Writes raw data to the stream.
         * @param [in] buffer Pointer to the beginning of the bufferi
         *                    containing the data to be written.
         * @param [in] count The number of bytes to write.
         * @throw Exception If write error occurred or if not all bytes were
         *                  written.
         */
        virtual void write(void *buffer,tuint32 count);
    };

    namespace canexstream
    {
        /**
         * Copies the contents of the input stream to the output stream. An
         * internal buffer is used to optimize the process. Progress is
         * reported through a Progresser object.
         * @param [in] from The source stream.
         * @param [in] to The target stream.
         * @param [in] progresser A reference to the progresser object to use
         *                        for reporting progress.
         * @throw Exception On read or write errors.
         */
        void copy(CanexInStream &from,CanexOutStream &to,Progresser &progresser);

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
         * @throw Exception On read or write errors.
         */
        void copy(CanexInStream &from,CanexOutStream &to,Progresser &progresser,
                  tuint64 size);
    }
}

