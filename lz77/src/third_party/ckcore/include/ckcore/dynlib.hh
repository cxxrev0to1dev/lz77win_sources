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
 * @file include/ckcore/dynlib.hh
 * @brief Defines a class for managing dynamic libraries.
 */

#pragma once
#ifdef _WINDOWS
#include <windows.h>
#endif
#include "ckcore/types.hh"
#include "ckcore/path.hh"

namespace ckcore
{
    /**
     * @brief Class for managing dynamic libraries.
     */
    class DynamicLibrary
    {
    private:
        Path path_;

#ifdef _WINDOWS
        HINSTANCE handle_;
#else
        void *handle_;
#endif

    public:
        /**
         * Constructs an DynamicLibrary object.
         * @param [in] path The path to the dynamic library.
         */
        DynamicLibrary(const Path &path);

        /**
         * Destructs the DynamicLibrary object.
         */
        ~DynamicLibrary();

        /**
         * Loads the dynamic library.
         * @return If the dynamic library was successfully loaded true is returned,
         *         otherwise false is returned.
         */
        bool open();

        /**
         * Unloads the dynamic library.
         * @return If the dynamic library was successfully unloaded true is
         *         returned, otherwise false is returned.
         */
        bool close();

        /**
         * Tests if the library is open.
         * @return If the library has been opened true is returned, if not false is
         *         returned.
         */
        bool test();

        /**
         * Returns a pointer to the specified symbol in the memory. On Windows the
         * symbol must be a procedure.
         * @param [in] symbol_name The name of the symbol.
         * @return The symbol memory address.
         */
        void *symbol(const char *symbol_name) const;

        /**
         * Returns the library path.
         * @return The library path.
         */
        const Path &path() const;
    };
}

