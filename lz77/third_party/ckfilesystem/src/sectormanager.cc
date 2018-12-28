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

#include "ckfilesystem/iso.hh"
#include "ckfilesystem/util.hh"
#include "ckfilesystem/sectormanager.hh"

namespace ckfilesystem
{
    SectorManager::SectorManager(ckcore::tuint64 start_sector) :
        next_free_sec_(start_sector),data_start_(0),data_len_(0)
    {
    }

    SectorManager::~SectorManager()
    {
    }

    /**
        Allocates a number of sectors for the client. The identifier is used
        for identifying the allocated sector range. This is necessary when the
        client requests which sector range it was given.
        @param client the client that requests the memory.
        @param identifier the unique identifier choosen by the client for the
        requested sector range.
        @param num_sec the number of sectors to allocate.
     */
    void SectorManager::alloc_sectors(SectorClient *client,unsigned char identifier,
                                      ckcore::tuint64 num_sec)
    {
        client_map_[std::make_pair(client,identifier)] = next_free_sec_;

        next_free_sec_ += num_sec;
    }

    void SectorManager::alloc_bytes(SectorClient *client,unsigned char identifier,
                                    ckcore::tuint64 num_bytes)
    {
        client_map_[std::make_pair(client,identifier)] = next_free_sec_;

        next_free_sec_ += util::bytes_to_sec64(num_bytes);
    }

    /**
        Allocation of data sectors is separated from the other allocation. The reason
        is that there can only be one data allocation and it should be accessible by
        any client.
     */
    void SectorManager::alloc_data_sectors(ckcore::tuint64 num_sec)
    {
        data_start_ = next_free_sec_;
        data_len_ = num_sec;

        next_free_sec_ += data_len_;
    }

    void SectorManager::alloc_data_bytes(ckcore::tuint64 num_bytes)
    {
        data_start_ = next_free_sec_;
        data_len_ = util::bytes_to_sec64(num_bytes);

        next_free_sec_ += data_len_;
    }

    /**
        Returns the starting sector of the allocated sector range allocated by
        the client that matches the identifier.
        @param client te client that owns the sector range.
        @param identifier the unique identifier selected by the client when
        allocating.
        @return the start sector.
     */
    ckcore::tuint64 SectorManager::get_start(SectorClient *client,unsigned char identifier)
    {
        return client_map_[std::make_pair(client,identifier)];
    }

    /**
        Returns the next free unallocated sector. This should be used with care.
     */
    ckcore::tuint64 SectorManager::get_next_free()
    {
        return next_free_sec_;
    }

    ckcore::tuint64 SectorManager::get_data_start()
    {
        return data_start_;
    }

    ckcore::tuint64 SectorManager::get_data_length()
    {
        return data_len_;
    }
};
