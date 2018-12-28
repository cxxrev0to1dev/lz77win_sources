/*
 * The ckCore library provides core software functionality.
 * Copyright (C) 2006-2012 Christian Kindahl
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file include/ckcore/file.hh
 * @brief Includes the platform specific file class.
 */

#pragma once

#ifdef _WINDOWS

#include <windows.h>

#elif defined(_UNIX)

// Nothing special for Unix.

#else
#error "Unknown platform"
#endif

#include <ckcore/types.hh>
#include <ckcore/path.hh>
#include <ckcore/exception.hh>

namespace ckcore
{

#ifdef _WINDOWS
#pragma warning(push)
#pragma warning(disable: 4290)  // C++ exception specification ignored except to...
#endif

    /**
     * @brief The class for dealing with files on Windows.
     */
    class File
    {
    public:
        /**
         * Defines modes which describes how to open files.
         */
        enum FileMode
        {
            ckOPEN_READ,
            ckOPEN_WRITE,
            ckOPEN_READWRITE
        };

        /**
         * Defines directives what to use as base offset when performing seek
         * operations.
         */
        enum FileWhence
        {
            ckFILE_CURRENT,
            ckFILE_BEGIN,
            ckFILE_END
        };

    private:
#ifdef _WINDOWS
        HANDLE file_handle_;
#else
        int file_handle_;
#endif
        Path file_path_;

        void check_file_is_open() const throw(std::exception);

    public:
        /**
         * Constructs a File object.
         * @param [in] file_path Path to the file.
         */
        File(const Path &file_path);
        ~File() { close(); }

        const tstring &name() const { return file_path_.name(); }

        /**
         * Opens the file in the requested mode.
         * @param [in] file_mode Determines how the file should be opened. In write
         *                       mode the file will be created if it does not
         *                       exist.
         * @return true if the file was successfully opened otherwise false is
         *         returned.
         */
        bool open(FileMode file_mode) throw();

        /**
         * Opens the file in the requested mode.
         * @param [in] file_mode Determines how the file should be opened. In write
         *                       mode the file will be created if it does not
         *                       exist.
         * @throw Exception object on error.
         */
        void open2(FileMode file_mode) throw(std::exception);

        /**
         * Closes the currently opened file handle. If the file has not been opened
         * a call this call will fail.
         * @return If successfull true is returned, otherwise false.
         */
        bool close();

        /**
         * Checks whether the file has been opened or not.
         * @return If a file is open true is returned, otherwise false is returned.
         */
        bool test() const;

        /**
         * Repositions the file pointer to the specified offset accoding to the
         * whence directive in the file.
         * @param [in] distance The number of bytes that the file pointer should
         *                      move.
         * @param [in] whence Specifies what to use as base when calculating the
         *                    final file pointer position.
         * @return If successfull the resulting file pointer location is returned,
         *         otherwise -1 is returned.
         */
        tint64 seek(tint64 distance,FileWhence whence) throw();

        /**
         * Repositions the file pointer to the specified offset accoding to the
         * whence directive in the file.
         * @param [in] distance The number of bytes that the file pointer should
         *                      move.
         * @param [in] whence Specifies what to use as base when calculating the
         *                    final file pointer position.
         * @return If successfull the resulting file pointer location is returned,
         *         otherwise an exception is thrown.
         * @throw Exception object on error.
         */
        tint64 seek2(tint64 distance,FileWhence whence) throw(std::exception);

        /**
         * Calculates the current file pointer position in the file.
         * @return If successfull the current file pointer position, otherwise -1
         *         is returned.
         */
        tint64 tell() const throw();

        /**
         * Calculates the current file pointer position in the file.
         * @return If successfull the current file pointer position, otherwise
         *         an exception is thrown.
         * @throw Exception object on error.
         */
        tint64 tell2() const throw(std::exception);

        /**
         * Reads raw data from the current file.
         * @param [out] buffer A pointer to the beginning of a buffer in which to
         *                     put the data.
         * @param [in] count The number of bytes to read from the file.
         * @return If the operation failed -1 is returned, otherwise the function
         *         returns the number of bytes read (this may be zero when the end
         *         of the file has been reached).
         */
        tint64 read(void *buffer,tint64 count);

        /**
         * Writes raw data to the current file.
         * @param [in] buffer A pointer to the beginning of a buffer from which to
         *                    read data to be written to the file.
         * @param [in] count The number of bytes to write to the file.
         * @return If the operation failed -1 is returned, otherwise the function
         *         returns the number of bytes written (this may be zero).
         */
        tint64 write(const void *buffer,tint64 count);

        /**
         * Checks whether the file exist or not.
         * @return If the file exist true is returned, otherwise false.
         */
        bool exist() const;

        /**
         * Removes the file from the file system. If other links exists to the file
         * only this link will be deleted. If the file is opened it will be closed.
         * @return If the file was successfully deleted true is returned, otherwise
         *         false is returned.
         */
        bool remove();

        /**
         * Moves the file to use the new file path. If the new full path exist it
         * will not be overwritten. If the file is open it will be closed.
         * @param [in] new_file_path The new file path.
         * @return If the file was sucessfully renamed true is returned, otherwise
         *         false is returned.
         */
        bool rename(const Path &new_file_path);

        /**
         * Obtains time stamps on when the file was last accessed, last modified
         * and created.
         * @param [out] access_time Time of last access.
         * @param [out] modify_time Time of last modification.
         * @param [out] create_time Time of creation (last status change on Linux).
         * @return If successfull true is returned, otherwise false.
         */
        bool time(struct tm &access_time,struct tm &modify_time,
                  struct tm &create_time) const;

        /**
         * Checks if the active user has permission to open the file in a
         * certain file mode.
         * @param [in] file_mode The file mode to check for access permission.
         * @return If the active user have permission to open the file in the
         *         specified file mode true is returned, otherwise false is
         *         returned.
         */
        bool access(FileMode file_mode) const { return access(file_path_,file_mode); }

        /**
         * Checks if the file is hidden or not.
         * @return If successfull and if the file is hidden true is returned,
         *         otherwise false is returned.
         */
        bool hidden() const { return hidden(file_path_); }

        /**
         * Calcualtes the size of the file.
         * @return If successfull the size of the file, otherwise -1 is returned.
         */
        tint64 size() throw();

        /**
         * Calculates the size of the file.
         * @return If successfull the size of the file, otherwise an exception
         *         is thrown.
         * @throw Exception object on error.
         */
        tint64 size2() throw(std::exception);

        /**
         * Checks whether the specified file exist or not.
         * @param [in] file_path The path to the file.
         * @return If the file exist true is returned, otherwise false.
         */
        static bool exist(const Path &file_path);

        /**
         * Removes the specified file from the file system. If other links exists
         * to the file only the specified link will be deleted.
         * @param [in] file_path The path to the file.
         * @return If the file was successfully deleted true is returned, otherwise
         *         false is returned.
         */
        static bool remove(const Path &file_path);

        /**
         * Moves the old file to use the new file path. If the new full path exist
         * it will not be overwritten.
         * @param [in] old_file_path The path to the file that should be moved.
         * @param [in] new_file_path The new path of the existing file.
         * @return If the file was sucessfully renamed true is returned, otherwise
         *         false is returned.
         */
        static bool rename(const Path &old_file_path,
                           const Path &new_file_path);

        /**
         * Obtains time stamps on when the specified file was last accessed, last
         * modified and created.
         * @param [in] file_path The path to the file.
         * @param [out] access_time Time of last access.
         * @param [out] modify_time Time of last modification.
         * @param [out] create_time Time of creation (last status change on Linux).
         * @return If successfull true is returned, otherwise false.
         */
        static bool time(const Path &file_path,struct tm &access_time,
                         struct tm &modify_time,struct tm &create_time);

        /**
         * Checks if the active user has permission to open the specified file in a
         * certain file mode.
         * @param [in] file_path The path to the file.
         * @param [in] file_mode The file mode to check for access permission.
         * @return If the active user have permission to open the file in the
         *         specified file mode true is returned, otherwise false is
         *         returned.
         */
        static bool access(const Path &file_path,FileMode file_mode);

        /**
         * Checks if the specified file is hidden or not.
         * @return If successfull and if the file is hidden true is returned,
         *         otherwise false is returned.
         */
        static bool hidden(const Path &file_path);

        /**
         * Calcualtes the size of the specified file.
         * @param [in] file_path The path to the file.
         * @return If successfull the size of the file, otherwise -1 is returned.
         */
        static tint64 size(const Path &file_path) throw();

        /**
         * Calculates the size of the specified file.
         * @param [in] file_path The path to the file.
         * @return If successfull the size of the file, otherwise an exception
         *         is thrown.
         * @throw Exception object on error.
         */
        static tint64 size2(const Path &file_path) throw(std::exception);

        /**
         * Creates a File object of a temporary file. The file path is generated
         * to be placed in the systems default temporary directory.
         * @param [in] prefix Prefix to use on temporary file name.
         * @return File object of temp file.
         */
        static File temp(const tchar *prefix);

        /**
         * Creates a File object of a temporary file. The file path is generated
         * to be placed in the specified path.
         * @param [in] file_path The path to where the temporary file should be
         *                       stored.
         * @param [in] prefix Prefix to use on temporary file name.
         * @return File object of temp file.
         */
        static File temp(const Path &file_path,const tchar *prefix);
    };

    /**
     * Opens the file in the requested mode.
     * @param [in] file_mode Determines how the file should be opened. In write
     *                       mode the file will be created if it does not
     *                       exist.
     * @return If successfull true is returned, otherwise false.
     */
    inline bool File::open(FileMode file_mode) throw()
    {
        try
        {
            open2(file_mode);
        }
        catch (...)
        {
            return false;
        }

        return true;
    }

    /**
     * Repositions the file pointer to the specified offset accoding to the
     * whence directive in the file.
     * @param [in] distance The number of bytes that the file pointer should
     *                      move.
     * @param [in] whence Specifies what to use as base when calculating the
     *                    final file pointer position.
     * @return If successfull the resulting file pointer location is returned,
     *         otherwise -1 is returned.
     */
    inline tint64 File::seek(tint64 distance,FileWhence whence) throw()
    {
        try
        {
            return seek2(distance,whence);
        }
        catch (...)
        {
            return -1;
        }
    }

    /**
     * Calculates the current file pointer position in the file.
     * @return If successfull the current file pointer position, otherwise -1
     *         is returned.
     */
    inline tint64 File::tell() const throw()
    {
        try
        {
            return tell2();
        }
        catch (...)
        {
            return -1;
        }
    }

    /**
     * Calculates the size of the file.
     * @return If successfull the size of the file, otherwise -1 is returned.
     */
    inline tint64 File::size() throw()
    {
        try
        {
            return size2();
        }
        catch (...)
        {
            return -1;
        }
    }

    /**
     * Calculates the size of the specified file.
     * @param [in] file_path The path to the file.
     * @return If successfull the size of the file, otherwise -1 is returned.
     */
    inline tint64 File::size(const Path &file_path) throw()
    {
        try
        {
            return size2(file_path);
        }
        catch (...)
        {
            return -1;
        }
    }

    inline void File::check_file_is_open() const throw(std::exception)
    {
        if (!test())
            throw Exception2(ckT("file not yet opened."));
    }

#ifdef _WINDOWS
#pragma warning(pop)
#endif
};
