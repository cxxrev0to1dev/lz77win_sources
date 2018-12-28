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
#include <string.h>
#include <time.h>
#include <string>
#include <ckcore/types.hh>
#include <ckcore/log.hh>
#include "ckfilesystem/const.hh"
#include "ckfilesystem/sectormanager.hh"
#include "ckfilesystem/fileset.hh"
#include "ckfilesystem/filetree.hh"
#include "ckfilesystem/iso.hh"
#include "ckfilesystem/isopathtable.hh"
#include "ckfilesystem/joliet.hh"
#include "ckfilesystem/eltorito.hh"
#include "ckfilesystem/filesystem.hh"

#define ISOWRITER_FILENAME_BUFFER_SIZE      206         // Must be enough to hold the largest possible string using
                                                        // any of the supported file system extensions.
namespace ckfilesystem
{
    class IsoImportData
    {
    public:
        unsigned char file_flags_;
        unsigned char file_unit_size_;
        unsigned char interleave_gap_size_;
        ckcore::tuint16 volseq_num_;
        ckcore::tuint32 extent_loc_;
        ckcore::tuint32 extent_len_;

        ckfilesystem::tiso_dir_record_datetime rec_timestamp_;

        IsoImportData() : file_flags_(0),file_unit_size_(0),
            interleave_gap_size_(0),volseq_num_(0),
            extent_loc_(0),extent_len_(0)
        {
            memset(&rec_timestamp_,0,sizeof(ckfilesystem::tiso_dir_record_datetime));
        }
    };

    class IsoWriter : public SectorClient
    {
    private:
        // Identifiers of different sector ranges.
        enum
        {
            SR_DESCRIPTORS,
            SR_BOOTCATALOG,
            SR_BOOTDATA,
            SR_PATHTABLE_NORMAL_L,
            SR_PATHTABLE_NORMAL_M,
            SR_PATHTABLE_JOLIET_L,
            SR_PATHTABLE_JOLIET_M,
            SR_DIRENTRIES
        };

        // Identifiers the two different types of system directories.
        enum SysDirType
        {
            TYPE_CURRENT,
            TYPE_PARENT
        };

        ckcore::Log &log_;
        SectorOutStream &out_stream_;
        SectorManager &sec_manager_;

        FileSystem &file_sys_;

        // File system attributes.
        bool use_joliet_;
        bool use_file_times_;

        // Sizes of different structures.
        ckcore::tuint64 pathtable_size_normal_;
        ckcore::tuint64 pathtable_size_joliet_;

        // The time when this object was created.
        struct tm create_time_;

        // File system preparation functions.
        void make_unique_joliet(FileTreeNode *node,unsigned char *file_name_ptr,
                                unsigned char file_name_size);
        void make_unique_iso(FileTreeNode *node,unsigned char *file_name_ptr,
                             unsigned char file_name_size);

        bool compare_strings(const char *str1,const ckcore::tchar *str2,
                             unsigned char len);
        bool compare_strings(const unsigned char *udf_str1,const ckcore::tchar *str2,
                             unsigned char len);

        bool calc_path_table_size(const IsoPathTable &pt,bool joliet_table,
                                  ckcore::tuint64 &pathtable_size,
                                  ckcore::Progress &progress);
        void calc_local_dir_entry_len(FileTreeNode *local_node,bool joliet,int level,
                                      ckcore::tuint32 &dir_len);
        void calc_local_dir_entries_len(std::vector<std::pair<FileTreeNode *,int> > &dir_node_stack,
                                        FileTreeNode *local_node,int level,
                                        ckcore::tuint64 &sec_offset);
        void calc_dir_entries_len(FileTree &file_tree,ckcore::tuint64 start_sec,
                                  ckcore::tuint64 &len);
        void calc_local_names(std::vector<FileTreeNode *> &node_stack,
                              FileTreeNode *node);

        // Write functions.
        void write_path_table(const IsoPathTable &pt,FileTree &file_tree,
                              bool joliet_table,bool msbf);
        void write_sys_dir(FileTreeNode *parent_node,SysDirType type,
                           ckcore::tuint32 data_pos,ckcore::tuint32 data_size);
        int write_local_dir_entry(ckcore::Progress &progress,FileTreeNode *local_node,
                                  bool joliet,int level);
        int write_local_dir_entries(std::vector<std::pair<FileTreeNode *,int> > &dir_node_stack,
                                    ckcore::Progress &progress,FileTreeNode *local_node,int level);

    public:
        IsoWriter(ckcore::Log &log,SectorOutStream &out_stream,SectorManager &sec_manager,
                      FileSystem &file_sys,bool use_file_times,bool use_joliet);
        ~IsoWriter();

        void calc_names(FileTree &file_tree);

        void alloc_header();
        void alloc_path_tables(const IsoPathTable &pt_iso,
                               const IsoPathTable &pt_jol,
                               ckcore::Progress &progress);
        void alloc_dir_entries(FileTree &file_tree);

        void write_header(const FileSet &files,FileTree &file_tree);
        void write_path_tables(const IsoPathTable &pt_iso,
                               const IsoPathTable &pt_jol,
                               FileTree &file_tree,ckcore::Progress &progress);
        int write_dir_entries(FileTree &file_tree,ckcore::Progress &progress);

        // Helper functions.
        bool validate_tree_node(std::vector<std::pair<FileTreeNode *,int> > &dir_node_stack,
                              FileTreeNode *node,int level);
        bool validate_tree(FileTree &file_tree);
    };
};
