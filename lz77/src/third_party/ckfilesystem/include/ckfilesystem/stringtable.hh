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

#pragma once
#include <map>
#include <ckcore/types.hh>

namespace ckfilesystem
{
    class StringTable
    {
    public:
        enum StringId
        {
            WARNING_FSDIRLEVEL,
            WARNING_SKIPFILE,
            WARNING_SKIP4GFILE,
            WARNING_SKIP4GFILEISO,
            ERROR_PATHTABLESIZE,    // FIXME: Not used.
            ERROR_OPENWRITE,
            ERROR_OPENREAD,
            STATUS_BUILDTREE,
            STATUS_WRITEDATA,
            STATUS_WRITEISOTABLE,
            STATUS_WRITEJOLIETTABLE,
            STATUS_WRITEDIRENTRIES,
            ERROR_DVDVIDEO
        };

    private:
        std::map<StringId,const ckcore::tchar *> strings_;

        StringTable();
        StringTable(const StringTable &obj);
        ~StringTable();
        StringTable &operator=(const StringTable &obj); 

    public:

        static StringTable &instance();

        const ckcore::tchar *get_string(StringId id);
        void set_string(StringId id,const ckcore::tchar *str);
    };
};
