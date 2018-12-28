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
#include <vector>
#include <string>
#include <ckcore/types.hh>
#include <ckcore/progress.hh>
#include <ckcore/log.hh>
#include <ckcore/stream.hh>
#include "ckfilesystem/fileset.hh"
#include "ckfilesystem/filetree.hh"
#include "ckfilesystem/sectorstream.hh"
#include "ckfilesystem/iso.hh"
#include "ckfilesystem/joliet.hh"
#include "ckfilesystem/eltorito.hh"
#include "ckfilesystem/udf.hh"
#include "ckfilesystem/filesystem.hh"

namespace ckfilesystem
{
    class FileSystemWriter
    {
    private:
        ckcore::Log &log_;          ///< Log object.
        FileSystem &file_sys_;      ///< What file system should be created.
        FileTree file_tree_;        ///< File tree for caching between the write and file_path_map functions.
        const bool fail_on_error_;  ///< Set to true in order to abort the operation if an error occurs.

        /**
         * Calculates file system specific data such as extent location and size for a
         * single file.
         * @throw Exception If advertised multi-session data can not be found.
         */
        void calc_local_filesys_data(std::vector<std::pair<FileTreeNode *,int> > &dir_node_stack,
                                     FileTreeNode *local_node,int level,ckcore::tuint64 &sec_offset,
                                     ckcore::Progress &progress);
        /**
         * Calculates file system specific data such as location of extents and sizes of
         * extents.
         * @throw Exception If advertised multi-session data can not be found.
         */
        void calc_filesys_data(FileTree &file_tree,ckcore::Progress &progress,
                               ckcore::tuint64 start_sec,ckcore::tuint64 &last_sec);

        void write_file_node(SectorOutStream &out_stream,FileTreeNode *node,
                             ckcore::Progresser &progresser);
        void write_local_file_data(SectorOutStream &out_stream,
                                   std::vector<std::pair<FileTreeNode *,int> > &dir_node_stack,
                                   FileTreeNode *local_node,int level,ckcore::Progresser &progresser);
        void write_file_data(SectorOutStream &out_stream,FileTree &file_tree,ckcore::Progresser &progresser);

        void get_internal_path(FileTreeNode *child_node,ckcore::tstring &node_path,
                               bool ext_path,bool joliet);
        void create_local_file_path_map(FileTreeNode *local_node,
                                        std::vector<FileTreeNode *> &dir_node_stack,
                                        std::map<ckcore::tstring,ckcore::tstring> &file_path_map,
                                        bool joliet);

        /**
         * Used for creating a map between the internal file names and the
         * external (Joliet or ISO9660, in that order).
         */
        void create_file_path_map(FileTree &file_tree,std::map<ckcore::tstring,ckcore::tstring> &file_path_map,
                                  bool joliet);

    public:
        FileSystemWriter(ckcore::Log &log,FileSystem &file_sys,bool fail_on_error);
        ~FileSystemWriter();    

        /**
         * Writes the file system to the specified output stream.
         * @param [out] out_stream Stream to write to.
         * @param [out] progress Object to report progress to.
         * @param [in] sec_offset Space assumed to be allocated before this
         *                        image, this is used for creating
         *                        multi-session discs.
         * @return RESULT_OK - on success.
         *         RESULT_FAIL - on faulure.
         *         RESULT_CANCEL - if the operation was cancelled by the user.
         */
        int write(ckcore::OutStream &out_stream,ckcore::Progress &progress,
                  ckcore::tuint32 sec_offset = 0);

        /**
         * @pre Must be called after the write function.
         */
        int file_path_map(std::map<ckcore::tstring,ckcore::tstring> &file_path_map);
    };
};
