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
 * @file include/ckcore/unix/directory.hh
 * @brief Defines the Unix directory class.
 */
#pragma once
#include <map>
#include <dirent.h>
#include "ckcore/file.hh"
#include "ckcore/path.hh"

namespace ckcore
{
    /**
     * @brief The class for dealing with directories on Unix.
     */
    class Directory
    {
    public:
        /**
         * @brief Class for iterating directory contents.
         *
         * Please note that each iterator (except for end iterators) allocates
         * an instance to the directory which will not be released until the
         * Directory (not Iterator) object is destroyed. In other words, a good
         * way of abusing this library is to create lots of iteators while
         * keeping the directory object alive.
         */
        class Iterator
        {
        private:
            DIR *dir_handle_;
            struct dirent *cur_ent_;

            void next();

        public:
            /**
             * Constructs an Iterator object.
             */
            Iterator();

            /**
             * Constructs an Iterator object.
             * @param [in] dir A reference to the Directory object to iterate over.
             */
            Iterator(const Directory &dir);

            /**
             * Returns the name of the file or directory that the iterator currently
             * points at.
             * @return The name of the file or directory that the iterator points at.
             */
            tstring operator*() const;

            /**
             * Moves the iterator to the next file or directory residing in the
             * directory.
             * @return An Iterator object pointing at the next file or directory.
             */
            Iterator &operator++();

            /**
             * Moves the iterator to the next file or directory residing in the
             * directory.
             * @return An Iterator object pointing at the next file or directory.
             */
            Iterator &operator++(int);

            /**
             * Tests the equivalence of this and another iterator.
             * @param [in] it The iterator to use for comparison.
             * @return If the iterators are equal true is returned, otherwise false.
             */
            bool operator==(const Iterator &it) const;

            /**
             * Tests the non-equivalence of this and another iterator.
             * @param [in] it The iterator to use for comparison.
             * @return If the iterators are equal true is returned, otherwise false.
             */
            bool operator!=(const Iterator &it) const;
        };

    private:
        Path dir_path_;

        std::map<Iterator *,DIR *> dir_handles_;

    public:
        /**
         * Constructs a Directory object.
         * @param [in] dir_path The path to the directory.
         */
        Directory(const Path &dir_path);

        /**
         * Destructs the Directory object.
         */
        ~Directory();

        /**
         * Returns the full directory path name.
         * @return The full directory path name.
         */
        const tstring &name() const;

        /**
         * Creates an iterator pointing to the first file or directory in the
         * current directory.
         * @return An Iteator object pointing to the first file or directory.
         */
        Iterator begin() const;

        /**
         * Creats an iterator poiting beyond the last file or directory in the
         * directory in the current directory.
         * @return An Iteator object pointing beyond the last file or directory.
         */
        Iterator end() const;

        /**
         * Creates the directory unless it already exist.
         * @return If successfull true is returned, otherwise false.
         */
        bool create() const;

        /**
         * Removes the directory if it exist.
         * @return If successfull true is returned, otherwise false.
         */
        bool remove() const;

        /**
         * Tests if the current directory exist.
         * @return If the directory exist true is returned, otherwise false.
         */
        bool exist() const;

        /**
         * Obtains time stamps on when the current directory was last accessed,
         * last modified and created.
         * @param [out] access_time Time of last access.
         * @param [out] modify_time Time of last modification.
         * @param [out] create_time Time of creation (last status change on Linux).
         * @return If successfull true is returned, otherwise false.
         */
        bool time(struct tm &access_time,struct tm &modify_time,
                  struct tm &create_time) const;

        /**
         * Creates the specified directory unless it already exist.
         * @return If successfull true is returned, otherwise false.
         */
        static bool create(const Path &dir_path);

        /**
         * Removes the specified directory if it exist.
         * @return If successfull true is returned, otherwise false.
         */
        static bool remove(const Path &dir_path);

        /**
         * Tests if the specified directory exist.
         * @return If the directory exist true is returned, otherwise false.
         */
        static bool exist(const Path &dir_path);

        /**
         * Obtains time stamps on when the specified directory was last accessed,
         * last modified and created.
         * @param [in] dir_path The path to the directory.
         * @param [out] access_time Time of last access.
         * @param [out] modify_time Time of last modification.
         * @param [out] create_time Time of creation (last status change on Linux).
         * @return If successfull true is returned, otherwise false.
         */
        static bool time(const Path &dir_path,struct tm &access_time,
                         struct tm &modify_time,struct tm &create_time);

        /**
         * Creates a Directory object describing a temporary directory on the hard
         * drive. The directory path is pointing to an unique directory name in the
         * default temporary directory of the current system. The directory is not
         * automatically created.
         * @return Directory object to a temporary directory.
         */
        static Directory temp();
    };
}

