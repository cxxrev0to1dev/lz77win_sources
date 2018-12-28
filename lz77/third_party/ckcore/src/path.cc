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

#include "ckcore/path.hh"

namespace ckcore
{
    Path::Iterator::Iterator() : at_end_(true),pos_start_(-1),pos_end_(0)
    {
    }

    Path::Iterator::Iterator(const Path &path) : at_end_(false),pos_start_(-1),
        pos_end_(0),path_(&path)
    {
        next();
    }

    void Path::Iterator::next()
    {
        if (pos_end_ == path_->path_name_.size() ||
            pos_end_ == path_->path_name_.size() - 1)
        {
            at_end_ = true;
            return;
        }
#ifdef _WINDOWS
        tstring::size_type delim = path_->path_name_.find_first_of(ckT("/\\"),pos_start_ + 1);

        if (delim == 0)
        {
            pos_start_++;
            delim = path_->path_name_.find_first_of(ckT("/\\"),pos_start_ + 1);
        }
#else
        tstring::size_type delim = path_->path_name_.find('/',pos_start_ + 1);

        if (delim == 0)
        {
            pos_start_++;
            delim = path_->path_name_.find('/',pos_start_ + 1);
        }
#endif

        if (delim == tstring::npos)
            pos_end_ = path_->path_name_.size();
        else
            pos_end_ = delim;
    }

    tstring Path::Iterator::operator*() const
    {
        if (at_end_)
            return tstring(ckT("NULL"));
        else
            return path_->path_name_.substr(pos_start_ + 1,pos_end_ - pos_start_ - 1);
    }

    Path::Iterator &Path::Iterator::operator++()
    {
        pos_start_ = pos_end_;
        next();

        return *this;
    }

    Path::Iterator &Path::Iterator::operator++(int)
    {
        pos_start_ = pos_end_;
        next();         

        return *this;
    }

    bool Path::Iterator::operator==(const Iterator &it) const
    {
        if (at_end_ && it.at_end_)
            return true;
        else
            return (**this) == *it;
    }

    bool Path::Iterator::operator!=(const Iterator &it) const
    {
        if (at_end_ && it.at_end_)
            return false;
        else
            return (**this) != *it;
    }

    Path::Path()
    {
    }

    Path::Path(const tchar *path_name) : path_name_(path_name)
    {
    }

    Path::~Path()
    {
    }

    Path::Iterator Path::begin() const
    {
        return Path::Iterator(*this);
    }

    Path::Iterator Path::end() const
    {
        return Path::Iterator();
    }

    bool Path::valid() const
    {
#ifdef _WINDOWS
        for (size_t i = 0; i < path_name_.size(); i++)
        {
            switch (path_name_[i])
            {
                case ':':
                    if (i != 1)
                        return false;
                    break;

                case '*':
                case '?':
                case '<':
                case '>':
                case '|':
                case '"':
                    return false;
            }
        }
#endif
        return true;
    }

    const tstring &Path::name() const
    {
        return path_name_;
    }

    tstring Path::root_name() const
    {
#ifdef _WINDOWS
        if (path_name_.size() > 2 && path_name_[1] == ':')
            return path_name_.substr(0,3);
#else
        if (path_name_.size() > 0 && path_name_[0] == '/')
            return tstring(ckT("/"));
#endif
        else
            return tstring();
    }

    tstring Path::dir_name() const
    {
        tstring::size_type end = path_name_.size() - 1;
#ifdef _WINDOWS
        tstring::size_type delim = path_name_.find_last_of(ckT("/\\"));
        if (delim == end)
            delim = path_name_.find_last_of(ckT("/\\"),end - 1);
#else
        tstring::size_type delim = path_name_.rfind('/');
        if (delim == end)
            delim = path_name_.rfind('/',end - 1);
#endif
        if (delim == tstring::npos)
            return tstring();
        else
            return path_name_.substr(0,delim + 1);
    }

    tstring Path::base_name() const
    {
        tstring::size_type end = path_name_.size() - 1;
#ifdef _WINDOWS
        tstring::size_type delim = path_name_.find_last_of(ckT("/\\"));
        if (delim == end)
            delim = path_name_.find_last_of(ckT("/\\"),--end);
#else
        tstring::size_type delim = path_name_.rfind('/');
        if (delim == end)
            delim = path_name_.rfind('/',--end);
#endif

        if (delim == tstring::npos)
            return path_name_.substr(0,end + 1);
        else
            return path_name_.substr(delim + 1,end - delim);
    }

    tstring Path::ext_name() const
    {
        tstring base_name = Path::base_name();
        tstring::size_type delim = base_name.rfind('.');
        if (delim == tstring::npos)
            return tstring();
        else
            return base_name.substr(delim + 1);
    }

    bool Path::operator==(const Path &p) const
    {
        Iterator it1 = begin();
        Iterator it2 = p.begin();

        while (it1 != end() && it2 != p.end())
        {
            if (it1 != it2)
                return false;

            it1++,it2++;
        }

        return it1 == end() && it2 == p.end();
    }

    bool Path::operator!=(const Path &p) const
    {
        return !(*this == p);
    }

    Path &Path::operator=(const Path &p)
    {
        path_name_ = p.path_name_;
        return *this;
    }

    Path Path::operator+(const Path &p) const
    {
        if (path_name_.size() < 1)
            return Path(p.path_name_.c_str());
        if (p.path_name_.size() < 1)
            return Path(p.path_name_.c_str());

        tstring new_path_name = path_name_;

        size_t end = new_path_name.size() - 1;
#ifdef _WINDOWS
        bool delim_p1 = new_path_name[end] == '/' || new_path_name[end] == '\\';
        bool delim_p2 = p.path_name_[0] == '/' || p.path_name_[0] == '\\';
#else
        bool delim_p1 = new_path_name[end] == '/';
        bool delim_p2 = p.path_name_[0] == '/';
#endif
        if (delim_p1 && delim_p2)
            new_path_name.resize(end);
        else if (!delim_p1 && !delim_p2)
            new_path_name.push_back('/');

        new_path_name += p.path_name_;
        return Path(new_path_name.c_str());
    }

    Path &Path::operator+=(const Path &p)
    {
        size_t end = path_name_.size() - 1;
#ifdef _WINDOWS
        bool delim_p1 = path_name_[end] == '/' || path_name_[end] == '\\';
        bool delim_p2 = p.path_name_[0] == '/' || p.path_name_[0] == '\\';
#else
        bool delim_p1 = path_name_[end] == '/';
        bool delim_p2 = p.path_name_[0] == '/';
#endif
        if (delim_p1 && delim_p2)
            path_name_.resize(end);
        else if (!delim_p1 && !delim_p2)
            path_name_.push_back('/');

        path_name_ += p.path_name_;
        return *this;
    }
}
