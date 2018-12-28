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
 * @file include/ckcore/path.hh
 * @brief Defines the path name class.
 */
#pragma once
#include <vector>
#include "ckcore/types.hh"

namespace ckcore
{
    /**
     * @brief Class for dealing with path names.
     */
    class Path
    {
    public:
        /**
         * @brief Class for iterating the file/directory structure of a path
         *        name.
         */
        class Iterator
        {
        private:
            bool at_end_;
            size_t pos_start_;
            size_t pos_end_;
            const Path *path_;

            void next();

        public:
            /**
             * Constructs an Interator object.
             */
            Iterator();

            /**
             * Constructs an Iterator object.
             * @param [in] path A reference to the Path object to iterate over.
             */
            Iterator(const Path &path);

            /**
             * Returns the name of the file or directory that the iterator currently
             * points at.
             * @return The name of the file or directory that the iterator points at.
             */
            tstring operator*() const;

            /**
             * Moves the iterator to the next file or directory in the path name.
             * @return An Iterator object pointing at the next file or directory name.
             */
            Iterator &operator++();

            /**
             * Moves the iterator to the next file or directory in the path name.
             * @return An Iterator object pointing at the next file or directory name.
             */
            Iterator &operator++(int);

            /**
             * Tests the equivalence of this and another iterator.
             * @param [in] it The iterator to use for comparison.
             * @return If the iterators are equal true is returned, otherwise false is
             *         returned.
             */
            bool operator==(const Iterator &it) const;

            /**
             * Tests the non-equivalence of this and another iterator.
             * @param [in] it The iterator to use for comparison.
             * @return If the iterators are not equal true is returned, otherwise
             *         false is returned.
             */
            bool operator!=(const Iterator &it) const;
        };

    private:
        tstring path_name_;

    public:
        /**
         * Constructs an empty Path object.
         */
        Path();

        /**
         * Constructs a Path object.
         * @param [in] path_name The path name.
         */
        Path(const tchar *path_name);

        /**
         * Destructs the Path object.
         */
        ~Path();

        /**
         * Creates an iterator pointing at the beginning of the path name.
         * @return An Iterator object pointing at the beginning of the path name.
         */
        Iterator begin() const;

        /**
         * Creates and iterator pointing at the end of the path name.
         * @return An Iterator object pointing at the end of the path name.
         */
        Iterator end() const;

        /**
         * Checks if the path name is valid, that it does not contain any invalid
         * characters.
         * @return If the path name is valid true is returned, otherwise false.
         */
        bool valid() const;

        /**
         * Returns the full path name.
         * @return The full path name.
         */
        const tstring &name() const;

        /**
         * Returns anything before and including the first path delimiter.
         * @return The path root name.
         */
        tstring root_name() const;

        /**
         * Calculates the dir name of the path name. The dir name will contain a
         * trailing path delimiter.
         * @return A string containing the dir name of the path name.
         */
        tstring dir_name() const;

        /**
         * Calculates the base name of the path name.
         * @return A string containing the base name of the path name.
         */
        tstring base_name() const;

        /**
         * Calculates the name of the file extension (if any). The separating dot
         * character is not included in the returned extension name.
         * @return A string containing the extension name.
         */
        tstring ext_name() const;

        /**
         * Tests the equivalence of this and another path.
         * @param [in] p The path to use for comparison.
         * @return If the two Path objects are equal true is returned, otherwise
         *         false.
         */
        bool operator==(const Path &p) const;

        /**
         * Tests the non-equivalence of this and another path.
         * @param [in] p The path to use for comparison.
         * @return If the two Path objects are not equal true is returned,
         *         otherwise false.
         */
        bool operator!=(const Path &p) const;

        /**
         * Assigns this path the value of another path.
         * @param [in] p The source path.
         * @return This path.
         */
        Path &operator=(const Path &p);

        /**
         * Concatinates this and another path.
         * @param [in] p The second path to use for concatination.
         * @return A new Path object containing the two paths concatinated.
         */
        Path operator+(const Path &p) const;

        /**
         * Append a path to this path.
         * @param [in] p The path to append.
         * @return This path.
         */
        Path &operator+=(const Path &p);
    };
}
