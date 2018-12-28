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
    class SectorClient
    {
    public:
        virtual ~SectorClient() {}
    };

    class SectorManager
    {
    private:
        ckcore::tuint64 next_free_sec_;
        ckcore::tuint64 data_start_;
        ckcore::tuint64 data_len_;
        std::map<std::pair<SectorClient *,unsigned char>,ckcore::tuint64> client_map_;

    public:
        SectorManager(ckcore::tuint64 start_sector);
        ~SectorManager();

        void alloc_sectors(SectorClient *client,unsigned char identifier,
                           ckcore::tuint64 num_sec);
        void alloc_bytes(SectorClient *client,unsigned char identifier,
                         ckcore::tuint64 num_bytes);

        void alloc_data_sectors(ckcore::tuint64 num_sec);
        void alloc_data_bytes(ckcore::tuint64 num_bytes);

        ckcore::tuint64 get_start(SectorClient *client,unsigned char identifier);
        ckcore::tuint64 get_next_free();

        ckcore::tuint64 get_data_start();
        ckcore::tuint64 get_data_length();
    };
};

