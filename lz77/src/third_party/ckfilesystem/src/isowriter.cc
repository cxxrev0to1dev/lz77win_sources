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

#include <time.h>
#include <string.h>
#include <stdio.h>
#include <wctype.h>
#include <ckcore/file.hh>
#include <ckcore/directory.hh>
#include <ckcore/convert.hh>
#include "ckfilesystem/util.hh"
#include "ckfilesystem/stringtable.hh"
#include "ckfilesystem/isoreader.hh"
#include "ckfilesystem/isowriter.hh"

namespace ckfilesystem
{
    using namespace util;

    IsoWriter::IsoWriter(ckcore::Log &log,SectorOutStream &out_stream,SectorManager &sec_manager,
                         FileSystem &file_sys,
                         bool use_file_times,bool use_joliet) :
        log_(log),out_stream_(out_stream),sec_manager_(sec_manager),
        file_sys_(file_sys),
        use_joliet_(use_joliet),use_file_times_(use_file_times),
        pathtable_size_normal_(0),pathtable_size_joliet_(0)
    {
        // Get system time.
        time_t cur_time;
        time(&cur_time);

        create_time_ = *localtime(&cur_time);
    }

    IsoWriter::~IsoWriter()
    {
    }

    /**
        Tries to assure that the specified file name is unqiue. Only 255 collisions
        are supported. If any more collisions occur duplicate file names will be
        written to the file system.
    */
    void IsoWriter::make_unique_joliet(FileTreeNode *node,unsigned char *file_name_ptr,
                                       unsigned char file_name_size)
    {
        FileTreeNode *parent_node = node->parent();
        if (parent_node == NULL)
            return;

        // Don't calculate a new name of one has already been generated.
        if (!node->file_name_joliet_.empty())
        {
            unsigned char file_name_pos = 0;
            for (unsigned int i = 0; i < file_name_size; i += 2,file_name_pos++)
            {
                file_name_ptr[i    ] = node->file_name_joliet_[file_name_pos] >> 8;
                file_name_ptr[i + 1] = node->file_name_joliet_[file_name_pos] & 0xff;
            }

            return;
        }

        unsigned char file_name_len = file_name_size >> 1;
        unsigned char file_name_pos = 0;

        wchar_t file_name[(ISOWRITER_FILENAME_BUFFER_SIZE >> 1) + 1];
        for (unsigned int i = 0; i < file_name_len; i++)
        {
            file_name[i]  = file_name_ptr[file_name_pos++] << 8;
            file_name[i] |= file_name_ptr[file_name_pos++];
        }

        file_name[file_name_len] = '\0';

        // We're only interested in the file name without the extension and
        // version information.
        int delim = -1;
        for (unsigned int i = file_name_len; i > 0; --i)
        {
            if (file_name[i - 1] == '.')
            {
                delim = i - 1;
                break;
            }
        }

        unsigned char file_name_end;

        // If no '.' character was found just remove the version information if
        // we're dealing with a file.
        if (delim == -1)
        {
            if (!(node->file_flags_ & FileTreeNode::FLAG_DIRECTORY) &&
                file_sys_.joliet_.includes_file_ver_info())
                file_name_end = file_name_len - 2;
            else
                file_name_end = file_name_len;
        }
        else
        {
            file_name_end = delim;
        }

        // We can't handle file names shorted than 3 characters (255 limit).
        if (file_name_end <= 3)
        {
            node->file_name_joliet_ = file_name;

            // Copy back to original buffer.
            file_name_pos = 0;
            for (unsigned int i = 0; i < file_name_size; i += 2,file_name_pos++)
            {
                file_name_ptr[i    ] = file_name[file_name_pos] >> 8;
                file_name_ptr[i + 1] = file_name[file_name_pos] & 0xff;
            }

            return;
        }

        unsigned char next_number = 2;
        wchar_t next_number_str[4];

        std::vector<FileTreeNode *>::const_iterator it_sibling;
        for (it_sibling = parent_node->children_.begin(); it_sibling != parent_node->children_.end();)
        {
            const FileTreeNode *sibling = *it_sibling;

            // Ignore any siblings that has not yet been assigned an ISO9660 compatible file name.
            if (sibling->file_name_iso_.empty())
            {
                it_sibling++;
                continue;
            }

            if (!wcscmp(sibling->file_name_joliet_.c_str(),file_name))
            {
                swprintf(next_number_str,4,L"%d",next_number);

                // Using if-statements for optimization.
                if (next_number < 10)
                    file_name[file_name_end - 1] = next_number_str[0];
                else if (next_number < 100)
                    memcpy(file_name + file_name_end - 2,next_number_str,2 * sizeof(wchar_t));
                else
                    memcpy(file_name + file_name_end - 3,next_number_str,3 * sizeof(wchar_t));

                if (next_number == 255)
                {
                    // We have failed, files with duplicate names will exist.
                    log_.print_line(ckT("  Warning: Unable to calculate unique Joliet name for %s. Duplicate file names will exist in Joliet name extension."),
                        node->file_path_.c_str());
                    break;
                }

                // Start from the beginning.
                next_number++;
                it_sibling = parent_node->children_.begin();
                continue;
            }

            it_sibling++;
        }

        node->file_name_joliet_ = file_name;

        // Copy back to original buffer.
        file_name_pos = 0;
        for (unsigned int i = 0; i < file_name_size; i += 2,file_name_pos++)
        {
            file_name_ptr[i    ] = file_name[file_name_pos] >> 8;
            file_name_ptr[i + 1] = file_name[file_name_pos] & 0xff;
        }
    }

    /**
        Tries to assure that the specified file name is unqiue. Only 255 collisions
        are supported. If any more collisions occur duplicate file names will be
        written to the file system.
    */
    void IsoWriter::make_unique_iso(FileTreeNode *node,unsigned char *file_name_ptr,
                                    unsigned char file_name_size)
    {
        FileTreeNode *parent_node = node->parent();
        if (parent_node == NULL)
            return;

        // Don't calculate a new name of one has already been generated.
        if (!node->file_name_iso_.empty())
        {
            memcpy(file_name_ptr,node->file_name_iso_.c_str(),file_name_size);
            return;
        }

        // We're only interested in modifying the file name without the extension
        // and version information.
        int delim = -1;
        for (unsigned int i = file_name_size; i > 0; --i)
        {
            if (file_name_ptr[i - 1] == '.')
            {
                delim = i - 1;
                break;
            }
        }

        unsigned char file_name_end = 0;
        unsigned char file_base_end = file_name_size;

        if (!(node->file_flags_ & FileTreeNode::FLAG_DIRECTORY) &&
                file_sys_.iso_.includes_file_ver_info())
        {
            file_base_end -= 2;
        }

        // If no '.' character was found just remove the version information if
        // we're dealing with a file.
        if (delim == -1)
        {
            if (!(node->file_flags_ & FileTreeNode::FLAG_DIRECTORY) &&
                file_sys_.iso_.includes_file_ver_info())
                file_name_end = file_name_size - 2;
            else
                file_name_end = file_name_size;
        }
        else
        {
            file_name_end = delim;
        }

        // We can't handle file names shorted than 3 characters (255 limit).
        if (file_name_end <= 3)
        {
            char file_name[ISOWRITER_FILENAME_BUFFER_SIZE + 1];
            memcpy(file_name,file_name_ptr,file_name_size);
            file_name[file_name_size] = '\0';

            node->file_name_iso_ = file_name;

            // Copy back to original buffer.
            memcpy(file_name_ptr,file_name,file_name_size);
            return;
        }

        unsigned char next_number = 2;
        char next_number_str[4];

        std::vector<FileTreeNode *>::const_iterator it_sibling;
        for (it_sibling = parent_node->children_.begin(); it_sibling != parent_node->children_.end();)
        {
            const FileTreeNode *sibling = *it_sibling;

            // Ignore any siblings that has not yet been assigned an ISO9660 compatible file name.
            if (sibling->file_name_iso_.empty())
            {
                it_sibling++;
                continue;
            }

            if (sibling->file_name_iso_.size() >= file_base_end &&
                !memcmp(sibling->file_name_iso_.c_str(),file_name_ptr,file_base_end))
            {
                sprintf(next_number_str,"%d",next_number);

                // Using if-statements for optimization.
                if (next_number < 10)
                    file_name_ptr[file_name_end - 1] = next_number_str[0];
                else if (next_number < 100)
                    memcpy(file_name_ptr + file_name_end - 2,next_number_str,2);
                else
                    memcpy(file_name_ptr + file_name_end - 3,next_number_str,3);

                if (next_number == 255)
                {
                    // We have failed, files with duplicate names will exist.
                    log_.print_line(ckT("  Warning: Unable to calculate unique ISO9660 name for %s. Duplicate file names will exist in ISO9660 file system."),
                        node->file_path_.c_str());
                    break;
                }

                // Start from the beginning.
                next_number++;
                it_sibling = parent_node->children_.begin();
                continue;
            }

            it_sibling++;
        }

        char file_name[ISOWRITER_FILENAME_BUFFER_SIZE + 1];
        memcpy(file_name,file_name_ptr,file_name_size);
        file_name[file_name_size] = '\0';

        node->file_name_iso_ = file_name;

        // Copy back to original buffer.
        memcpy(file_name_ptr,file_name,file_name_size);
    }

    bool IsoWriter::compare_strings(const char *str1,const ckcore::tchar *str2,
                                    unsigned char len)
    {
#ifdef _WINDOWS
#ifdef _UNICODE
        for (unsigned char i = 0; i < len; i++)
            if (toupper(str1[i]) != toupper(str2[i]))
                return false;

        return true;
#else
        return !_strnicmp(str1,str2,len);
#endif
#else   
        return !strncasecmp(str1,str2,len);
#endif
    }

    bool IsoWriter::compare_strings(const unsigned char *udf_str1,
                                    const ckcore::tchar *str2,
                                    unsigned char len)
    {
#ifdef _UNICODE
        unsigned char file_name_pos = 0;
        for (unsigned int i = 0; i < len; i++)
        {
            wchar_t c = udf_str1[file_name_pos++] << 8;
            c |= udf_str1[file_name_pos++];

            if (towupper(c) != towupper(str2[i]))
                return false;
        }

        return true;
#else
        unsigned char file_name_pos = 0;
        for (unsigned int i = 0; i < len; i++)
        {
            wchar_t c = udf_str1[file_name_pos++] << 8;
            c |= udf_str1[file_name_pos++];

            if (towupper(c) != wint_t(toupper(str2[i])))
                return false;
        }
        return true;
#endif
    }

    bool IsoWriter::calc_path_table_size(const IsoPathTable &pt,bool joliet_table,
                                         ckcore::tuint64 &pathtable_size,
                                         ckcore::Progress &progress)
    {
        // Root record + 1 padding byte since the root record size is odd.
        pathtable_size = sizeof(tiso_pathtable_record) + 1;

        // Write all other path table records.
        IsoPathTable::const_iterator it;
        for (it = pt.begin(); it != pt.end(); it++)
        {
            FileTreeNode *cur_node = it->first;

            // FIXME: This can be made to use the length of the already computed names.
            unsigned char name_len = joliet_table ?
                file_sys_.joliet_.calc_file_name_len(cur_node->file_name_.c_str(),true) << 1 : 
                file_sys_.iso_.calc_file_name_len(cur_node->file_name_.c_str(),true);
            unsigned char pathtable_rec_len = sizeof(tiso_pathtable_record) + name_len - 1;

            // If the record length is not even padd it with a 0 byte.
            if (pathtable_rec_len % 2 == 1)
                pathtable_rec_len++;

            // Update the path table length.
            pathtable_size += pathtable_rec_len;
        }

        return true;
    }

    /*
        Calculates the size of a directory entry in sectors. This size does not include the size
        of the extend data of the directory contents.
    */
    void IsoWriter::calc_local_dir_entry_len(FileTreeNode *local_node,bool joliet,
                                             int level,ckcore::tuint32 &dir_len)
    {
        dir_len = 0;

        // The number of bytes of data in the current sector.
        // Each directory always includes '.' and '..'.
        ckcore::tuint32 dir_sec_data = sizeof(tiso_dir_record) << 1;

        std::vector<FileTreeNode *>::const_iterator it_file;
        for (it_file = local_node->children_.begin(); it_file !=
            local_node->children_.end(); it_file++)
        {
            if ((*it_file)->file_flags_ & FileTreeNode::FLAG_DIRECTORY)
            {
                // Validate directory level.
                if (level > file_sys_.iso_.get_max_dir_level())
                    continue;
            }

            // Validate file size.
            if ((*it_file)->file_size_ > ISO_MAX_EXTENT_SIZE && !file_sys_.iso_.allows_fragmentation())
                continue;

            // Calculate the number of times this record will be written. It
            // will be larger than one when using multi-extent.
            ckcore::tuint32 factor = 1;
            ckcore::tuint64 extent_remain = (*it_file)->file_size_;
            while (extent_remain > ISO_MAX_EXTENT_SIZE)
            {
                extent_remain -= ISO_MAX_EXTENT_SIZE;
                factor++;
            }

            bool is_folder = (*it_file)->file_flags_ & FileTreeNode::FLAG_DIRECTORY;

            unsigned char name_size;
            
            // Optimization: If the the compatible file name is equal (not case-sensitive) to the
            // original file name we assume that it is uniqe.
            unsigned char file_name[ISOWRITER_FILENAME_BUFFER_SIZE + 4]; // Large enough for level 1, 2 and even Joliet.
            if (joliet)
            {
                name_size = file_sys_.joliet_.write_file_name(file_name,(*it_file)->file_name_.c_str(),is_folder) << 1;

                unsigned char file_name_end;
                if (!is_folder && file_sys_.joliet_.includes_file_ver_info())
                    file_name_end = (name_size >> 1) - 2;
                else
                    file_name_end = (name_size >> 1);

                // UPDATE: Removed due to causing duplicate Joliet file names.
                /*if (compare_strings(file_name,(*it_file)->file_name_.c_str(),file_name_end))
                {
#ifdef _UNICODE
                    (*it_file)->file_name_joliet_ = (*it_file)->file_name_;

                    if (!is_folder && file_sys_.joliet_.includes_file_ver_info())
                        (*it_file)->file_name_joliet_.append(L";1");
#else
                    wchar_t szWideFileName[(ISOWRITER_FILENAME_BUFFER_SIZE >> 1) + 1];
                    unsigned char file_name_pos = 0;
                    for (unsigned char i = 0; i < (name_len >> 1); i++)
                    {
                        szWideFileName[i]  = file_name[file_name_pos++] << 8;
                        szWideFileName[i] |= file_name[file_name_pos++];
                    }

                    szWideFileName[name_len >> 1] = '\0';

                    (*it_file)->file_name_joliet_ = szWideFileName;
#endif
                }*/
            }
            else
            {
                name_size = file_sys_.iso_.write_file_name(file_name,(*it_file)->file_name_.c_str(),is_folder);

                unsigned char file_name_end;
                if (!is_folder && file_sys_.iso_.includes_file_ver_info())
                    file_name_end = name_size - 2;
                else
                    file_name_end = name_size;

                // UPDATE: Removed due to causing duplicate ISO9660 file names.
                /*if (compare_strings((const char *)file_name,(*it_file)->file_name_.c_str(),file_name_end))
                {
                    file_name[name_len] = '\0';
                    (*it_file)->file_name_iso_ = (const char *)file_name;
                }*/
            }

            unsigned char cur_rec_size = sizeof(tiso_dir_record) + name_size - 1;

            // If the record length is not even padd it with a 0 byte.
            if (cur_rec_size % 2 == 1)
                cur_rec_size++;

            if (dir_sec_data + (cur_rec_size * factor) > ISO_SECTOR_SIZE)
            {
                dir_sec_data = cur_rec_size * factor;
                dir_len++;
            }
            else
            {
                dir_sec_data += cur_rec_size * factor;
            }
        }

        if (dir_sec_data != 0)
            dir_len++;
    }

    void IsoWriter::calc_local_dir_entries_len(std::vector<std::pair<FileTreeNode *,int> > &dir_node_stack,
                                               FileTreeNode *local_node,int level,
                                               ckcore::tuint64 &sec_offset)
    {
        ckcore::tuint32 dir_len_normal = 0;
        calc_local_dir_entry_len(local_node,false,level,dir_len_normal);

        ckcore::tuint32 dir_len_joliet = 0;
        if (use_joliet_)
            calc_local_dir_entry_len(local_node,true,level,dir_len_joliet);

        std::vector<FileTreeNode *>::const_iterator it_file;
        for (it_file = local_node->children_.begin(); it_file !=
            local_node->children_.end(); it_file++)
        {
            if ((*it_file)->file_flags_ & FileTreeNode::FLAG_DIRECTORY)
            {
                // Validate directory level.
                if (level > file_sys_.iso_.get_max_dir_level())
                    continue;
                else
                    dir_node_stack.push_back(std::make_pair(*it_file,level + 1));
            }
        }

        // We now know how much space is needed to store the current directory record.
        local_node->data_size_normal_ = dir_len_normal * ISO_SECTOR_SIZE;
        local_node->data_size_joliet_ = dir_len_joliet * ISO_SECTOR_SIZE;

        local_node->data_pos_normal_ = sec_offset;
        sec_offset += dir_len_normal;
        local_node->data_pos_joliet_ = sec_offset;
        sec_offset += dir_len_joliet;
    }

    void IsoWriter::calc_dir_entries_len(FileTree &file_tree,
                                         ckcore::tuint64 start_sec,
                                         ckcore::tuint64 &len)
    {
        FileTreeNode *cur_node = file_tree.get_root();
        ckcore::tuint64 sec_offset = start_sec;

        std::vector<std::pair<FileTreeNode *,int> > dir_node_stack;
        calc_local_dir_entries_len(dir_node_stack,cur_node,2,sec_offset);

        while (dir_node_stack.size() > 0)
        { 
            cur_node = dir_node_stack[dir_node_stack.size() - 1].first;
            int level = dir_node_stack[dir_node_stack.size() - 1].second;
            dir_node_stack.pop_back();

            calc_local_dir_entries_len(dir_node_stack,cur_node,level,sec_offset);
        }

        len = sec_offset - start_sec;
    }

    void IsoWriter::calc_local_names(std::vector<FileTreeNode *> &node_stack,
                                     FileTreeNode *node)
    {
        std::vector<FileTreeNode *>::const_iterator it;
        for (it = node->children_.begin(); it !=
            node->children_.end(); it++)
        {
            FileTreeNode *cur_node = *it;
            bool is_folder = cur_node->file_flags_ & FileTreeNode::FLAG_DIRECTORY;

            if (is_folder)
                node_stack.push_back(*it);

            unsigned char name_size;
            unsigned char file_name[ISOWRITER_FILENAME_BUFFER_SIZE];    // Large enough for both level 1, 2 and even Joliet.

            if (file_sys_.is_iso())
            {
                name_size = file_sys_.iso_.write_file_name(file_name,cur_node->file_name_.c_str(),is_folder);
                make_unique_iso(cur_node,file_name,name_size);
            }

            if (file_sys_.is_joliet())
            {
                name_size = file_sys_.joliet_.write_file_name(file_name,cur_node->file_name_.c_str(),is_folder) << 1;
                make_unique_joliet(cur_node,file_name,name_size);
            }
        }
    }

    void IsoWriter::calc_names(FileTree &file_tree)
    {
        FileTreeNode *cur_node = file_tree.get_root();

        std::vector<FileTreeNode *> node_stack;
        calc_local_names(node_stack,cur_node);

        while (node_stack.size() > 0)
        { 
            cur_node = node_stack.back();
            node_stack.pop_back();

            calc_local_names(node_stack,cur_node);
        }
    }

    void IsoWriter::write_path_table(const IsoPathTable &pt,FileTree &file_tree,
                                     bool joliet_table,bool msbf)
    {
        FileTreeNode *cur_node = file_tree.get_root();

        tiso_pathtable_record root_record,path_record;
        memset(&root_record,0,sizeof(root_record));

        root_record.dir_ident_len = 1;                  // Always 1 for the root record.
        root_record.ext_attr_record_len = 0;

        if (joliet_table)
        {
            if (msbf)
                write732(root_record.extent_loc,(ckcore::tuint32)cur_node->data_pos_joliet_);   // Start sector of extent data.
            else
                write731(root_record.extent_loc,(ckcore::tuint32)cur_node->data_pos_joliet_);   // Start sector of extent data.
        }
        else
        {
            if (msbf)
                write732(root_record.extent_loc,(ckcore::tuint32)cur_node->data_pos_normal_);   // Start sector of extent data.
            else
                write731(root_record.extent_loc,(ckcore::tuint32)cur_node->data_pos_normal_);   // Start sector of extent data.
        }

        if (msbf)
            write722(root_record.parent_dir_num,0x01);  // The root has itself as parent.
        else
            write721(root_record.parent_dir_num,0x01);  // The root has itself as parent.

        root_record.dir_ident[0] = 0;                   // The file name is set to zero.

        // Write the root record.
        out_stream_.write(&root_record,sizeof(root_record));

        // We need to pad the root record since it's size is otherwise odd.
        out_stream_.write(root_record.dir_ident,1);

        // Write all other path table records.
        IsoPathTable::const_iterator it;
        for (it = pt.begin(); it != pt.end(); it++)
        {
            cur_node = it->first;

            memset(&path_record,0,sizeof(path_record));

            unsigned char name_size;
            unsigned char file_name[ISOWRITER_FILENAME_BUFFER_SIZE];    // Large enough for both level 1, 2 and even Joliet.

            if (joliet_table)
            {
                name_size = file_sys_.joliet_.write_file_name(file_name,cur_node->file_name_.c_str(),true) << 1;
                make_unique_joliet(cur_node,file_name,name_size);
            }
            else
            {
                name_size = file_sys_.iso_.write_file_name(file_name,cur_node->file_name_.c_str(),true);
                make_unique_iso(cur_node,file_name,name_size);
            }

            // If the record length is not even padd it with a 0 byte.
            bool pad_byte = false;
            if (name_size % 2 == 1)
                pad_byte = true;

            path_record.dir_ident_len = name_size;
            path_record.ext_attr_record_len = 0;

            if (joliet_table)
            {
                if (msbf)
                    write732(path_record.extent_loc,(ckcore::tuint32)cur_node->data_pos_joliet_);
                else
                    write731(path_record.extent_loc,(ckcore::tuint32)cur_node->data_pos_joliet_);
            }
            else
            {
                if (msbf)
                    write732(path_record.extent_loc,(ckcore::tuint32)cur_node->data_pos_normal_);
                else
                    write731(path_record.extent_loc,(ckcore::tuint32)cur_node->data_pos_normal_);
            }

            if (msbf)
                write722(path_record.parent_dir_num,it->second);
            else
                write721(path_record.parent_dir_num,it->second);

            path_record.dir_ident[0] = 0;

            // Write the record.
            out_stream_.write(&path_record,sizeof(path_record) - 1);
            out_stream_.write(file_name,name_size);

            // Pad if necessary.
            if (pad_byte)
            {
                char szTemp[1] = { 0 };
                out_stream_.write(szTemp,1);
            }
        }

        if (out_stream_.get_allocated() != 0)
            out_stream_.pad_sector();
    }

    void IsoWriter::write_sys_dir(FileTreeNode *parent_node,SysDirType type,
                                  ckcore::tuint32 data_pos,ckcore::tuint32 data_size)
    {
        tiso_dir_record dr;
        memset(&dr,0,sizeof(dr));

        dr.dir_record_len = 0x22;
        write733(dr.extent_loc,data_pos);
        write733(dr.data_len,data_size);
        iso_make_datetime(create_time_,dr.rec_timestamp);
        dr.file_flags = DIRRECORD_FILEFLAG_DIRECTORY;
        write723(dr.volseq_num,0x01);   // The directory is on the first volume set.
        dr.file_ident_len = 1;
        dr.file_ident[0] = type == TYPE_CURRENT ? 0 : 1;

        out_stream_.write(&dr,sizeof(dr));
    }

    bool IsoWriter::validate_tree_node(std::vector<std::pair<FileTreeNode *,int> > &dir_node_stack,
                                       FileTreeNode *node,int level)
    {
        std::vector<FileTreeNode *>::const_iterator it_file;
        for (it_file = node->children_.begin(); it_file !=
            node->children_.end(); it_file++)
        {
            if ((*it_file)->file_flags_ & FileTreeNode::FLAG_DIRECTORY)
            {
                if (level > file_sys_.iso_.get_max_dir_level())
                    return false;
                else
                    dir_node_stack.push_back(std::make_pair(*it_file,level + 1));
            }
        }

        return true;
    }

    /*
        Returns true if the specified file tree conforms to the configuration
        used to create the disc image.
    */
    bool IsoWriter::validate_tree(FileTree &file_tree)
    {
        FileTreeNode *cur_node = file_tree.get_root();

        std::vector<std::pair<FileTreeNode *,int> > dir_node_stack;
        if (!validate_tree_node(dir_node_stack,cur_node,2))
            return false;

        while (dir_node_stack.size() > 0)
        { 
            cur_node = dir_node_stack[dir_node_stack.size() - 1].first;
            int level = dir_node_stack[dir_node_stack.size() - 1].second;
            dir_node_stack.pop_back();

            if (!validate_tree_node(dir_node_stack,cur_node,level))
                return false;
        }

        return true;
    }

    void IsoWriter::alloc_header()
    {
        // Allocate volume descriptor.
        ckcore::tuint32 voldesc_size = sizeof(tiso_voldesc_primary) + sizeof(tiso_voldesc_setterm);
        if (file_sys_.eltorito_.get_boot_image_count() > 0)
            voldesc_size += sizeof(tiso_voldesc_eltorito_record);
        if (file_sys_.iso_.has_vol_desc_suppl())
            voldesc_size += sizeof(tiso_voldesc_suppl);
        if (use_joliet_)
            voldesc_size += sizeof(tiso_voldesc_suppl);

        sec_manager_.alloc_bytes(this,SR_DESCRIPTORS,voldesc_size);

        // Allocate boot catalog and data.
        if (file_sys_.eltorito_.get_boot_image_count() > 0)
        {
            sec_manager_.alloc_bytes(this,SR_BOOTCATALOG,file_sys_.eltorito_.get_boot_cat_size());
            sec_manager_.alloc_bytes(this,SR_BOOTDATA,file_sys_.eltorito_.get_boot_data_size());
        }
    }

    void IsoWriter::alloc_path_tables(const IsoPathTable &pt_iso,
                                      const IsoPathTable &pt_jol,
                                      ckcore::Progress &progress)
    {
        // Calculate path table sizes.
        pathtable_size_normal_ = 0;
        if (!calc_path_table_size(pt_iso,false,pathtable_size_normal_,progress))
            throw ckcore::Exception2(ckT("Unable to calculate path table size."));

        pathtable_size_joliet_ = 0;
        if (use_joliet_ && !calc_path_table_size(pt_jol,true,pathtable_size_joliet_,progress))
            throw ckcore::Exception2(ckT("Unable to calculate joliet path table size."));

        if (pathtable_size_normal_ > 0xffffffff || pathtable_size_joliet_ > 0xffffffff)
        {
            ckcore::tstringstream msg;
            msg << ckT("The path table is too large, ") << pathtable_size_normal_
                << ckT(" and ") << pathtable_size_joliet_ << ckT(" bytes.");
            throw ckcore::Exception2(msg.str());
        }

        sec_manager_.alloc_bytes(this,SR_PATHTABLE_NORMAL_L,pathtable_size_normal_);
        sec_manager_.alloc_bytes(this,SR_PATHTABLE_NORMAL_M,pathtable_size_normal_);
        sec_manager_.alloc_bytes(this,SR_PATHTABLE_JOLIET_L,pathtable_size_joliet_);
        sec_manager_.alloc_bytes(this,SR_PATHTABLE_JOLIET_M,pathtable_size_joliet_);
    }

    void IsoWriter::alloc_dir_entries(FileTree &file_tree)
    {
        ckcore::tuint64 dir_entries_len = 0;
        calc_dir_entries_len(file_tree,sec_manager_.get_next_free(),dir_entries_len);

        sec_manager_.alloc_sectors(this,SR_DIRENTRIES,dir_entries_len);

#ifdef _WINDOWS
        log_.print_line(ckT("  Allocated directory entries %I64u sectors."),dir_entries_len);
#else
        log_.print_line(ckT("  Allocated directory entries %llu sectors."),dir_entries_len);
#endif
    }

    void IsoWriter::write_header(const FileSet &files,FileTree &file_tree)
    {
        // Make sure that everything has been allocated.
        if (pathtable_size_normal_ == 0)
            throw ckcore::Exception2(ckT("Memory for ISO9660 path table has not been allocated."));

        // Calculate boot catalog and image data.
        if (file_sys_.eltorito_.get_boot_image_count() > 0)
        {
            ckcore::tuint64 boot_data_sec = sec_manager_.get_start(this,SR_BOOTDATA);
            ckcore::tuint64 boot_data_len = bytes_to_sec64(file_sys_.eltorito_.get_boot_data_size());
            ckcore::tuint64 boot_data_end = boot_data_sec + boot_data_len;

            if (boot_data_sec > 0xffffffff || boot_data_end > 0xffffffff)
            {
                ckcore::tstringstream msg;
                msg << ckT("Invalid boot data sector range of ")
                    << boot_data_sec << ckT(" to ")
                    << boot_data_end << ckT(".");
                throw ckcore::Exception2(msg.str());
            }

            file_sys_.eltorito_.calc_filesys_data(boot_data_sec,boot_data_end);
        }

        ckcore::tuint32 pos_pathtable_normal_l = (ckcore::tuint32)sec_manager_.get_start(this,SR_PATHTABLE_NORMAL_L);
        ckcore::tuint32 pos_pathtable_normal_m = (ckcore::tuint32)sec_manager_.get_start(this,SR_PATHTABLE_NORMAL_M);
        ckcore::tuint32 pos_pathtable_joliet_l = (ckcore::tuint32)sec_manager_.get_start(this,SR_PATHTABLE_JOLIET_L);
        ckcore::tuint32 pos_pathtable_joliet_m = (ckcore::tuint32)sec_manager_.get_start(this,SR_PATHTABLE_JOLIET_M);

        // Print the sizes.
        ckcore::tuint64 dir_entries_sec = sec_manager_.get_start(this,SR_DIRENTRIES);
        ckcore::tuint64 file_data_end_sec = sec_manager_.get_data_start() + sec_manager_.get_data_length();

        if (dir_entries_sec > 0xffffffff)
        {
            ckcore::tstringstream msg;
            msg << ckT("Invalid start sector ") << dir_entries_sec
                << ckT(" of directory entries.");
            throw ckcore::Exception2(msg.str());
        }

        file_sys_.iso_.write_vol_desc_primary(out_stream_,create_time_,
                                              (ckcore::tuint32)file_data_end_sec,
                                              (ckcore::tuint32)pathtable_size_normal_,
                                              pos_pathtable_normal_l,pos_pathtable_normal_m,
                                              (ckcore::tuint32)dir_entries_sec,
                                              (ckcore::tuint32)file_tree.get_root()->data_size_normal_);

        // Write the El Torito boot record at sector 17 if necessary.
        if (file_sys_.eltorito_.get_boot_image_count() > 0)
        {
            ckcore::tuint64 boot_cat_sec = sec_manager_.get_start(this,SR_BOOTCATALOG);
            file_sys_.eltorito_.write_boot_record(out_stream_,(ckcore::tuint32)boot_cat_sec);

#ifdef _WINDOWS
            log_.print_line(ckT("  Wrote El Torito boot record at sector %I64u."),boot_cat_sec);
#else
            log_.print_line(ckT("  Wrote El Torito boot record at sector %llu."),boot_cat_sec);
#endif
        }

        // Write ISO9660 descriptor.
        if (file_sys_.iso_.has_vol_desc_suppl())
        {
            file_sys_.iso_.write_vol_desc_suppl(out_stream_,create_time_,
                                                (ckcore::tuint32)file_data_end_sec,
                                                (ckcore::tuint32)pathtable_size_normal_,
                                                pos_pathtable_normal_l,
                                                pos_pathtable_normal_m,
                                                (ckcore::tuint32)dir_entries_sec,
                                                (ckcore::tuint32)file_tree.get_root()->data_size_normal_);
        }

        // Write the Joliet header.
        if (use_joliet_)
        {
            if (file_tree.get_root()->data_size_normal_ > 0xffffffff)
            {
                ckcore::tstringstream msg;
                msg << ckT("Root folder is larger than ISO9660 supports, current size is ")
                    << file_tree.get_root()->data_size_normal_ << ckT(" bytes.");
                throw ckcore::Exception2(msg.str());
            }

            ckcore::tuint32 root_extent_loc_joliet = (ckcore::tuint32)dir_entries_sec +
                bytes_to_sec((ckcore::tuint32)file_tree.get_root()->data_size_normal_);

            file_sys_.joliet_.write_vol_desc(out_stream_,create_time_,
                                             (ckcore::tuint32)file_data_end_sec,
                                             (ckcore::tuint32)pathtable_size_joliet_,
                                             pos_pathtable_joliet_l,pos_pathtable_joliet_m,
                                             root_extent_loc_joliet,
                                             (ckcore::tuint32)file_tree.get_root()->data_size_joliet_);
        }

        file_sys_.iso_.write_vol_desc_setterm(out_stream_);

        // Write the El Torito boot catalog and boot image data.
        if (file_sys_.eltorito_.get_boot_image_count() > 0)
        {
            file_sys_.eltorito_.write_boot_catalog(out_stream_);
            file_sys_.eltorito_.write_boot_images(out_stream_);
        }
    }

    void IsoWriter::write_path_tables(const IsoPathTable &pt_iso,
                                      const IsoPathTable &pt_jol,
                                      FileTree &file_tree,ckcore::Progress &progress)
    {
        progress.set_status(ckT("%s"),  StringTable::instance().get_string(StringTable::STATUS_WRITEISOTABLE));

        // Write the path tables.
        write_path_table(pt_iso,file_tree,false,false);
        write_path_table(pt_iso,file_tree,false,true);

        if (use_joliet_)
        {
            progress.set_status(ckT("%s"), StringTable::instance().get_string(StringTable::STATUS_WRITEJOLIETTABLE));

            write_path_table(pt_jol,file_tree,true,false);
            write_path_table(pt_jol,file_tree,true,true);
        }
    }

    int IsoWriter::write_local_dir_entry(ckcore::Progress &progress,
                                         FileTreeNode *local_node,
                                         bool joliet,int level)
    {
        tiso_dir_record dr;

        // Write the '.' and '..' directories.
        FileTreeNode *parent_node = local_node->parent();
        if (parent_node == NULL)
            parent_node = local_node;

        if (joliet)
        {
            write_sys_dir(local_node,TYPE_CURRENT,
                (ckcore::tuint32)local_node->data_pos_joliet_,
                (ckcore::tuint32)local_node->data_size_joliet_);
            write_sys_dir(local_node,TYPE_PARENT,
                (ckcore::tuint32)parent_node->data_pos_joliet_,
                (ckcore::tuint32)parent_node->data_size_joliet_);
        }
        else
        {
            write_sys_dir(local_node,TYPE_CURRENT,
                (ckcore::tuint32)local_node->data_pos_normal_,
                (ckcore::tuint32)local_node->data_size_normal_);
            write_sys_dir(local_node,TYPE_PARENT,
                (ckcore::tuint32)parent_node->data_pos_normal_,
                (ckcore::tuint32)parent_node->data_size_normal_);
        }

        // The number of bytes of data in the current sector.
        // Each directory always includes '.' and '..'.
        ckcore::tuint32 dir_sec_data = sizeof(tiso_dir_record) << 1;

        std::vector<FileTreeNode *>::const_iterator it_file;
        for (it_file = local_node->children_.begin(); it_file !=
            local_node->children_.end(); it_file++)
        {
            // Check if we should abort.
            if (progress.cancelled())
                return RESULT_CANCEL;

            if ((*it_file)->file_flags_ & FileTreeNode::FLAG_DIRECTORY)
            {
                // Validate directory level.
                if (level > file_sys_.iso_.get_max_dir_level())
                    continue;
            }

            // Validate file size.
            if ((*it_file)->file_size_ > ISO_MAX_EXTENT_SIZE && !file_sys_.iso_.allows_fragmentation())
                continue;

            // This loop is necessary for multi-extent support.
            ckcore::tuint64 file_remain = joliet ? (*it_file)->data_size_joliet_ : (*it_file)->data_size_normal_;
            ckcore::tuint64 extent_loc = joliet ? (*it_file)->data_pos_joliet_ : (*it_file)->data_pos_normal_;

            do
            {
                // We can't actually use 0xffffffff bytes since that will not fit perfectly within a sector range.
                ckcore::tuint32 extent_size = file_remain > ISO_MAX_EXTENT_SIZE ?
                    ISO_MAX_EXTENT_SIZE : (ckcore::tuint32)file_remain;

                memset(&dr,0,sizeof(dr));

                // Make a valid file name.
                unsigned char name_size;
                unsigned char file_name[ISOWRITER_FILENAME_BUFFER_SIZE + 4]; // Large enough for level 1, 2 and even Joliet.

                bool is_folder = (*it_file)->file_flags_ & FileTreeNode::FLAG_DIRECTORY;
                if (joliet)
                {
                    name_size = file_sys_.joliet_.write_file_name(file_name,(*it_file)->file_name_.c_str(),is_folder) << 1;
                    make_unique_joliet((*it_file),file_name,name_size);
                }
                else
                {
                    name_size = file_sys_.iso_.write_file_name(file_name,(*it_file)->file_name_.c_str(),is_folder);
                    make_unique_iso((*it_file),file_name,name_size);
                }

                // If the record length is not even pad it with a 0 byte.
                bool pad_byte = false;
                unsigned char dir_rec_size = sizeof(dr) + name_size - 1;
                if (dir_rec_size % 2 == 1)
                {
                    pad_byte = true;
                    dir_rec_size++;
                }

                dr.dir_record_len = dir_rec_size;   // FIXME: Rename member.
                dr.ext_attr_record_len = 0;

                write733(dr.extent_loc,(ckcore::tuint32)extent_loc);
                write733(dr.data_len,(ckcore::tuint32)extent_size);

                if ((*it_file)->file_flags_ & FileTreeNode::FLAG_IMPORTED)
                {
                    IsoImportData *import_node = (IsoImportData *)(*it_file)->data_ptr_;
                    if (import_node == NULL)
                    {
                        log_.print_line(ckT("  Error: The file \"%s\" does not contain imported session data like advertised."),
                            (*it_file)->file_name_.c_str());
                        return RESULT_FAIL;
                    }

                    memcpy(&dr.rec_timestamp,&import_node->rec_timestamp_,sizeof(tiso_dir_record_datetime));

                    dr.file_flags = import_node->file_flags_;
                    dr.file_unit_size = import_node->file_unit_size_;
                    dr.interleave_gap_size = import_node->interleave_gap_size_;
                    write723(dr.volseq_num,import_node->volseq_num_);
                }
                else
                {
                    // Date time.
                    if (use_file_times_)
                    {
                        struct tm access_time,modify_time,create_time;
                        bool res = true;

                        if ((*it_file)->file_flags_ & FileTreeNode::FLAG_DIRECTORY)
                        {
                            res = ckcore::Directory::time((*it_file)->file_path_.c_str(),
                                                          access_time,modify_time,create_time);
                            
                        }
                        else
                        {
                            res = ckcore::File::time((*it_file)->file_path_.c_str(),
                                                     access_time,modify_time,create_time);
                        }

                        ckcore::tuint16 file_date = 0,file_time = 0;
                        ckcore::convert::tm_to_dostime(modify_time,file_date,file_time);

                        if (res)
                            iso_make_datetime(file_date,file_time,dr.rec_timestamp);
                        else
                            iso_make_datetime(create_time_,dr.rec_timestamp);
                    }
                    else
                    {
                        // The time when the disc image creation was initialized.
                        iso_make_datetime(create_time_,dr.rec_timestamp);
                    }

                    // File flags.
                    dr.file_flags = 0;
                    if ((*it_file)->file_flags_ & FileTreeNode::FLAG_DIRECTORY)
                        dr.file_flags |= DIRRECORD_FILEFLAG_DIRECTORY;

                    if (ckcore::File::hidden((*it_file)->file_path_.c_str()))
                        dr.file_flags |= DIRRECORD_FILEFLAG_HIDDEN;

                    dr.file_unit_size = 0;
                    dr.interleave_gap_size = 0;
                    write723(dr.volseq_num,0x01);
                }

                // Remaining bytes, before checking if we're dealing with the last segment.
                file_remain -= extent_size;
                if ((*it_file)->file_size_ > ISO_MAX_EXTENT_SIZE && file_remain > 0)
                    dr.file_flags |= DIRRECORD_FILEFLAG_MULTIEXTENT;

                dr.file_ident_len = name_size;

                // Pad the sector with zeros if we can not fit the complete
                // directory entry on this sector.
                if ((dir_sec_data + dir_rec_size) > ISO_SECTOR_SIZE)
                {
                    dir_sec_data = dir_rec_size;
                    
                    // Pad the sector with zeros.
                    out_stream_.pad_sector();
                }
                else if ((dir_sec_data + dir_rec_size) == ISO_SECTOR_SIZE)
                {
                    dir_sec_data = 0;
                }
                else
                {
                    dir_sec_data += dir_rec_size;
                }

                // Write the record.
                out_stream_.write(&dr,sizeof(dr) - 1);
                out_stream_.write(file_name,name_size);

                // Pad if necessary.
                if (pad_byte)
                {
                    char szTemp[1] = { 0 };
                    out_stream_.write(szTemp,1);
                }

                // Update location of the next extent.
                extent_loc += bytes_to_sec(extent_size);
            }
            while (file_remain > 0);
        }

        if (out_stream_.get_allocated() != 0)
            out_stream_.pad_sector();

        return RESULT_OK;
    }

    int IsoWriter::write_local_dir_entries(std::vector<std::pair<FileTreeNode *,int> > &dir_node_stack,
                                           ckcore::Progress &progress,FileTreeNode *local_node,int level)
    {
        std::vector<FileTreeNode *>::const_iterator it_file;
        for (it_file = local_node->children_.begin(); it_file !=
            local_node->children_.end(); it_file++)
        {
            if ((*it_file)->file_flags_ & FileTreeNode::FLAG_DIRECTORY)
            {
                // Validate directory level.
                if (level > file_sys_.iso_.get_max_dir_level())
                    continue;
                else
                    dir_node_stack.push_back(std::make_pair(*it_file,level + 1));
            }
        }

        int res = write_local_dir_entry(progress,local_node,false,level);
        if (res != RESULT_OK)
            return res;

        if (use_joliet_)
        {
            res = write_local_dir_entry(progress,local_node,true,level);
            if (res != RESULT_OK)
                return res;
        }

        return RESULT_OK;
    }

    int IsoWriter::write_dir_entries(FileTree &file_tree,ckcore::Progress &progress)
    {
        progress.set_status(ckT("%s"), StringTable::instance().get_string(StringTable::STATUS_WRITEDIRENTRIES));

        FileTreeNode *cur_node = file_tree.get_root();

        std::vector<std::pair<FileTreeNode *,int> > dir_node_stack;
        if (!write_local_dir_entries(dir_node_stack,progress,cur_node,2))
            return RESULT_FAIL;

        while (dir_node_stack.size() > 0)
        { 
            cur_node = dir_node_stack[dir_node_stack.size() - 1].first;
            int level = dir_node_stack[dir_node_stack.size() - 1].second;
            dir_node_stack.pop_back();

            int res = write_local_dir_entries(dir_node_stack,progress,cur_node,level);
            if (res != RESULT_OK)
                return res;
        }

        return RESULT_OK;
    }
};
