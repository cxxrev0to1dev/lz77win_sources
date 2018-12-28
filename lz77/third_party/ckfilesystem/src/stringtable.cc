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

#include "ckfilesystem/stringtable.hh"

namespace ckfilesystem
{
    StringTable::StringTable()
    {
        strings_[WARNING_FSDIRLEVEL] = ckT("The directory structure is deeper than %d levels. Deep files and folders will be ignored.");
        strings_[WARNING_SKIPFILE] = ckT("Skipping \"%s\".");
        strings_[WARNING_SKIP4GFILE] = ckT("Skipping \"%s\", the file is larger than 4 GiB.");
        strings_[WARNING_SKIP4GFILEISO] = ckT("The file \"%s\" is larger than 4 GiB. It will not be visible in the ISO9660/Joliet file system.");
        strings_[ERROR_PATHTABLESIZE] = ckT("The disc image path table is to large. The project contains too many files.");
        strings_[ERROR_OPENWRITE] = ckT("Unable to open file for writing: %s.");
        strings_[ERROR_OPENREAD] = ckT("Unable to open file for reading: %s.");
        strings_[STATUS_BUILDTREE] = ckT("Building file tree.");
        strings_[STATUS_WRITEDATA] = ckT("Writing file data.");
        strings_[STATUS_WRITEISOTABLE] = ckT("Writing ISO9660 path tables.");
        strings_[STATUS_WRITEJOLIETTABLE] = ckT("Writing Joliet path tables.");
        strings_[STATUS_WRITEDIRENTRIES] = ckT("Writing directory entries.");
        strings_[ERROR_DVDVIDEO] = ckT("Cannot create DVD-Video file system, is the VIDEO_TS folder present?");
    }

    StringTable::~StringTable()
    {
    }

    StringTable &StringTable::instance()
    {
        static StringTable instance;
        return instance;
    }

    const ckcore::tchar *StringTable::get_string(StringId id)
    {
        return strings_[id];
    }

    /*
        For translation purposes.
    */
    void StringTable::set_string(StringId id,const ckcore::tchar *str)
    {
        strings_[id] = str;
    }
};
