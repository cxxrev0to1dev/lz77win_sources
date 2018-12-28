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

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include "ckcore/convert.hh"
#include "ckcore/directory.hh"

namespace ckcore
{
    Directory::Iterator::Iterator() : dir_handle_(NULL),cur_ent_(NULL)
    {
    }

    Directory::Iterator::Iterator(const Directory &dir) : dir_handle_(NULL),
        cur_ent_(NULL)
    {
        if (dir.dir_handles_.count(this) > 0)
        {
            dir_handle_ = const_cast<Directory &>(dir).dir_handles_[this];
        }
        else
        {
            dir_handle_ = opendir(dir.dir_path_.name().c_str());
            const_cast<Directory &>(dir).dir_handles_[this] = dir_handle_;
        }

        if (dir_handle_ != NULL)
            next();
    }

    void Directory::Iterator::next()
    {
        cur_ent_ = readdir(dir_handle_);

        // Skip system '.' and '..' directories.
        while ((cur_ent_ != NULL) && (!strcmp(cur_ent_->d_name,".") ||
                                      !strcmp(cur_ent_->d_name,"..")))
        {
            cur_ent_ = readdir(dir_handle_);
        }
    }

    tstring Directory::Iterator::operator*() const
    {
        if (cur_ent_ == NULL)
            return tstring("NULL");
        else
            return tstring(cur_ent_->d_name);
    }

    Directory::Iterator &Directory::Iterator::operator++()
    {
        if (cur_ent_ != NULL)
            next();

        return *this;
    }

    Directory::Iterator &Directory::Iterator::operator++(int)
    {
        if (cur_ent_ != NULL)
            next();

        return *this;
    }

    bool Directory::Iterator::operator==(const Iterator &it) const
    {
        if (cur_ent_ == NULL && it.cur_ent_ == NULL)
            return true;

        if ((cur_ent_ == NULL && it.cur_ent_ != NULL) ||
            (cur_ent_ != NULL && it.cur_ent_ == NULL))
            return false;

        return !strcmp(cur_ent_->d_name,it.cur_ent_->d_name);
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
        std::map<Iterator *,DIR *>::iterator it;
        for (it = dir_handles_.begin(); it != dir_handles_.end(); it++)
            closedir(it->second);

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
        return rmdir(dir_path_.name().c_str()) == 0;
    }

    bool Directory::exist() const
    {
        struct stat file_stat;
        if (stat(dir_path_.name().c_str(),&file_stat) != 0)
            return false;

        return (file_stat.st_mode & S_IFDIR) > 0;
    }

    bool Directory::time(struct tm &access_time,struct tm &modify_time,
                         struct tm &create_time) const
    {
        return time(dir_path_,access_time,modify_time,create_time);
    }

    bool Directory::create(const Path &dir_path)
    {
        tstring cur_path = dir_path.root_name();

        Path::Iterator it;
        for (it = dir_path.begin(); it != dir_path.end(); it++)
        {
            cur_path += *it + "/";
            if (!exist(cur_path.c_str()))
            {
                if (mkdir(cur_path.c_str(),S_IRUSR | S_IWUSR | S_IXUSR) != 0)
                    return false;
            }
        }

        return true;
    }

    bool Directory::remove(const Path &dir_path)
    {
        return rmdir(dir_path.name().c_str()) == 0;
    }

    bool Directory::exist(const Path &dir_path)
    {
        struct stat file_stat;
        if (stat(dir_path.name().c_str(),&file_stat) != 0)
            return false;

        return (file_stat.st_mode & S_IFDIR) > 0;
    }

    bool Directory::time(const Path &dir_path,struct tm &access_time,
                         struct tm &modify_time,struct tm &create_time)
    {
        struct stat dir_stat;
        if (stat(dir_path.name().c_str(),&dir_stat) == -1)
            return false;

        // Convert to local time.
        if (localtime_r(&dir_stat.st_atime,&access_time) == NULL)
            return false;

        if (localtime_r(&dir_stat.st_mtime,&modify_time) == NULL)
            return false;

        if (localtime_r(&dir_stat.st_ctime,&create_time) == NULL)
            return false;

        return true;
    }

    Directory Directory::temp()
    {
        tchar *tmp_name = tmpnam(NULL);
        if (tmp_name != NULL)
        {
            return Directory(tmp_name);
        }
        else
        {
            tchar tmp_name2[PATH_MAX+1];
            strcpy(tmp_name2,ckT("/tmp/file"));

            tchar convBuffer[convert::INT_TO_STR_BUFLEN];
            convert::ui32_to_str2(rand(), convBuffer);
            strcat(tmp_name2, convBuffer);
            
            strcat(tmp_name2,ckT(".tmp"));

            return Directory(tmp_name2);
        }
    }
}
