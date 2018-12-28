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
#include <ckcore/log.hh>
#include "ckfilesystem/fileset.hh"
#include "ckfilesystem/filetree.hh"
#include "ckfilesystem/iforeader.hh"

#define DVDVIDEO_BLOCK_SIZE         2048

namespace ckfilesystem
{
    class DvdVideo
    {
    private:
        enum FileSetType
        {
            FST_INFO,
            FST_BACKUP,
            FST_MENU,
            FST_TITLE
        };

        ckcore::Log &log_;

        ckcore::tuint64 size_to_dvd_len(ckcore::tuint64 file_size);

        FileTreeNode *find_video_node(FileTree &file_tree,FileSetType type,ckcore::tuint32 number);

        bool get_total_titles_size(ckcore::tstring &file_path,FileSetType type,ckcore::tuint32 number,
                                   ckcore::tuint64 &file_size);
        bool read_file_set_info_root(FileTree &file_tree,IfoVmgData &vmg_data,
                                     std::vector<ckcore::tuint32> &ts_sectors);
        bool read_file_set_info(FileTree &file_tree,std::vector<ckcore::tuint32> &ts_sectors);

    public:
        DvdVideo(ckcore::Log &log);
        ~DvdVideo();

        bool print_file_padding(FileTree &file_tree);

        bool calc_file_padding(FileTree &file_tree);
    };
};

