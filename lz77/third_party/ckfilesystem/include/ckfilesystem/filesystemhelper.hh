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
#include <ckcore/types.hh>
#include "ckfilesystem/filesystemwriter.hh"
#include "ckfilesystem/iso.hh"
#include "ckfilesystem/joliet.hh"

namespace ckfilesystem
{
    class FileSystemHelper
    {
    private:
        FileSystem &file_sys_;

    public:
        FileSystemHelper(FileSystem &file_sys);
        ~FileSystemHelper();

        void calc_file_name(const ckcore::tchar *req_file_name,ckcore::tchar *file_name,bool is_dir);
        void calc_file_path(const ckcore::tchar *req_file_path,ckcore::tstring &file_path);
    };
};
