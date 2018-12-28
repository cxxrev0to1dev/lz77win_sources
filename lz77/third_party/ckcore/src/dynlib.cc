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

#ifdef _WINDOWS
#else
#include <dlfcn.h>
#endif
#include "ckcore/dynlib.hh"

namespace ckcore
{
    DynamicLibrary::DynamicLibrary(const Path &path) : path_(path),handle_(NULL)
    {
    }

    DynamicLibrary::~DynamicLibrary()
    {
    }

    bool DynamicLibrary::open()
    {
        if (handle_ != NULL)
            return false;

#ifdef _WINDOWS
        handle_ = LoadLibrary(path_.name().c_str());
#else
        handle_ = dlopen(path_.name().c_str(),RTLD_NOW);
#endif
        return handle_ != NULL;
    }

    bool DynamicLibrary::close()
    {
        if (handle_ == NULL)
            return false;

#ifdef _WINDOWS
        if (FreeLibrary(handle_) == FALSE)
            return false;
#else
        if (dlclose(handle_) == -1)
            return false;
#endif
        handle_ = NULL;
        return true;
    }

    bool DynamicLibrary::test()
    {
        return handle_ != NULL;
    }

    void *DynamicLibrary::symbol(const char *symbol_name) const
    {
        if (handle_ == NULL)
            return false;

#ifdef _WINDOWS
        return static_cast<void *>(GetProcAddress(handle_,symbol_name));
#else
        return dlsym(handle_,symbol_name);
#endif
    }

    const Path &DynamicLibrary::path() const
    {
        return path_;
    }
}

