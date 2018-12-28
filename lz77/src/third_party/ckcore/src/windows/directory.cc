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

#include "stdafx.hh"
#include "util.hh"
#include "ckcore/convert.hh"
#include "ckcore/directory.hh"

namespace ckcore
{
    Directory::Iterator::Iterator() : dir_handle_(INVALID_HANDLE_VALUE),
        at_end_(true)
    {
    }

    Directory::Iterator::Iterator(const Directory &dir) :
        dir_handle_(INVALID_HANDLE_VALUE),at_end_(false)
    {
        if (dir.dir_handles_.count(this) > 0)
        {
            dir_handle_ = const_cast<Directory &>(dir).dir_handles_[this];
        }
        else
        {
            tstring path = dir.dir_path_.name();
            tchar last = path[path.size() - 1];

            // Make sure we have a trailing delimiter.
            if (last != '/' && last != '\\')
                path += '/';
            
            path += '*';

            dir_handle_ = FindFirstFile(path.c_str(),&cur_ent_);
            const_cast<Directory &>(dir).dir_handles_[this] = dir_handle_;
        }
        
        if (dir_handle_ == INVALID_HANDLE_VALUE)
        {
            at_end_ = true;
        }
        else
        {
            // Skip system '.' and '..' directories.
            if (!lstrcmp(cur_ent_.cFileName,ckT(".")) ||
                !lstrcmp(cur_ent_.cFileName,ckT("..")))
            {
                next();
            }
        }
    }

    void Directory::Iterator::next()
    {
        while (!(at_end_ = !FindNextFile(dir_handle_,&cur_ent_)))
        {
            // Skip system '.' and '..' directories.
            if (lstrcmp(cur_ent_.cFileName,ckT(".")) &&
                lstrcmp(cur_ent_.cFileName,ckT("..")))
            {
                break;
            }
        }
    }

    tstring Directory::Iterator::operator*() const
    {
        if (at_end_)
            return tstring(ckT("NULL"));
        else
            return tstring(cur_ent_.cFileName);
    }

    Directory::Iterator &Directory::Iterator::operator++()
    {
        if (!at_end_)
            next();

        return *this;
    }

    Directory::Iterator &Directory::Iterator::operator++(int)
    {
        if (!at_end_)
            next();

        return *this;
    }

    bool Directory::Iterator::operator==(const Iterator &it) const
    {
        if (at_end_ && it.at_end_)
            return true;

        if ((at_end_ && !it.at_end_) ||
            (!at_end_ && it.at_end_))
            return false;

        return !lstrcmp(cur_ent_.cFileName,it.cur_ent_.cFileName);
    }

    bool Directory::Iterator::operator!=(const Iterator &it) const
    {
        return !(*this == it);
    }

    Directory::Directory(const Path &dir_path) : dir_path_(dir_path)
    {
    }

    Directory::~Directory()
    {
        // Since the Directory object owns the iterator handles, we need to
        // free them.
        std::map<Iterator *,HANDLE>::iterator it;
        for (it = dir_handles_.begin(); it != dir_handles_.end(); it++)
            FindClose(it->second);

        dir_handles_.clear();
    }

    const tstring &Directory::name() const
    {
        return dir_path_.name();
    }

    Directory::Iterator Directory::begin() const
    {
        return Directory::Iterator(*this);
    }

    Directory::Iterator Directory::end() const
    {
        return Directory::Iterator();
    }

    bool Directory::create() const
    {
        return create(dir_path_);
    }

    bool Directory::remove() const
    {
        return RemoveDirectory(dir_path_.name().c_str()) != FALSE;
    }

    bool Directory::exist() const
    {
        unsigned long attr = GetFileAttributes(dir_path_.name().c_str());

        return (attr != -1) && (attr & FILE_ATTRIBUTE_DIRECTORY);
    }

    bool Directory::time(struct tm &accessckTime,struct tm &modifyckTime,
                         struct tm &createckTime) const
    {
        return time(dir_path_,accessckTime,modifyckTime,createckTime);
    }

    bool Directory::create(const Path &dir_path)
    {
        tstring cur_path;

        Path::Iterator it;
        for (it = dir_path.begin(); it != dir_path.end(); it++)
        {
            cur_path += *it + ckT("/");
            if (!exist(cur_path.c_str()))
            {
                if (CreateDirectory(cur_path.c_str(),NULL) == FALSE)
                    return false;
            }
        }

        return true;
    }

    bool Directory::remove(const Path &dir_path)
    {
        return RemoveDirectory(dir_path.name().c_str()) != FALSE;
    }

    bool Directory::exist(const Path &dir_path)
    {
        unsigned long attr = GetFileAttributes(dir_path.name().c_str());

        return (attr != -1) && (attr & FILE_ATTRIBUTE_DIRECTORY);
    }

    bool Directory::time(const Path &dir_path,struct tm &accessckTime,
                         struct tm &modifyckTime,struct tm &createckTime)
    {
        WIN32_FILE_ATTRIBUTE_DATA file_info;
        if (GetFileAttributesEx(dir_path.name().c_str(),GetFileExInfoStandard,
                                &file_info) == FALSE)
        {
            return false;
        }

        // Convert to system time.
        SYSTEMTIME access_stime,modify_stime,create_stime;

        if (FileTimeToSystemTime(&file_info.ftLastAccessTime,&access_stime) == FALSE)
            return false;

        if (FileTimeToSystemTime(&file_info.ftLastWriteTime,&modify_stime) == FALSE)
            return false;

        if (FileTimeToSystemTime(&file_info.ftCreationTime,&create_stime) == FALSE)
            return false;

        // Convert to struct tm.
        SysTimeToTm(access_stime,accessckTime);
        SysTimeToTm(modify_stime,modifyckTime);
        SysTimeToTm(create_stime,createckTime);

        return true;
    }

    Directory Directory::temp()
    {
        tchar dir_name[246];
        if (GetTempPath(sizeof(dir_name) / sizeof(tchar),dir_name) == 0)
            dir_name[0] = '\0';

        tchar tmp_name[_MAX_PATH];
        if (GetTempFileName(dir_name,ckT("tmp"),0,tmp_name) == 0)
        {
            // Fall back on random name generation.
            lstrcpy(tmp_name,dir_name);
            lstrcat(tmp_name,ckT("file"));

            tchar convBuffer[convert::INT_TO_STR_BUFLEN];
            convert::ui32_to_str2((tuint32)rand(), convBuffer);
            lstrcat(tmp_name,convBuffer);
                                 
            lstrcat(tmp_name,ckT(".tmp"));
        }

        Path tmp_path(tmp_name);

        if (File::exist(tmp_path))
            File::remove(tmp_path);

        return Directory(tmp_path);
    }
};
