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

#include <ckcore/string.hh>
#include "ckfilesystem/stringtable.hh"
#include "ckfilesystem/sectormanager.hh"
#include "ckfilesystem/isowriter.hh"
#include "ckfilesystem/udfwriter.hh"
#include "ckfilesystem/dvdvideo.hh"
#include "ckfilesystem/exception.hh"
#include "ckfilesystem/filesystemwriter.hh"

namespace ckfilesystem
{
    FileSystemWriter::FileSystemWriter(ckcore::Log &log,FileSystem &file_sys,
                                       bool fail_on_error) :
        log_(log),file_sys_(file_sys),file_tree_(log),fail_on_error_(fail_on_error)
    {
    }

    FileSystemWriter::~FileSystemWriter()
    {
    }

    void FileSystemWriter::calc_local_filesys_data(std::vector<std::pair<FileTreeNode *,int> > &dir_node_stack,
                                                   FileTreeNode *local_node,int level,
                                                   ckcore::tuint64 &sec_offset,ckcore::Progress &progress)
    {
        std::vector<FileTreeNode *>::const_iterator it_file;
        for (it_file = local_node->children_.begin(); it_file !=
            local_node->children_.end(); it_file++)
        {
            if ((*it_file)->file_flags_ & FileTreeNode::FLAG_DIRECTORY)
            {
                // Validate directory level.
                if (level > file_sys_.get_max_dir_level())
                    continue;
                else
                    dir_node_stack.push_back(std::make_pair(*it_file,level + 1));
            }
            else
            {
                // Validate file size.
                if ((*it_file)->file_size_ > ISO_MAX_EXTENT_SIZE && !file_sys_.allows_fragmentation())
                {
                    bool is_iso = file_sys_.is_iso(),is_udf = file_sys_.is_udf();

                    // FIXME: Make nested loops.
                    if (is_iso && !is_udf)
                    {
                        log_.print_line(ckT("  Warning: Skipping \"%s\", the file is larger than 4 GiB."),
                            (*it_file)->file_name_.c_str());
                        progress.notify(ckcore::Progress::ckWARNING,StringTable::instance().get_string(StringTable::WARNING_SKIP4GFILE),
                            (*it_file)->file_name_.c_str());

                        continue;
                    }
                    else if (is_iso && is_udf)
                    {
                        log_.print_line(ckT("  Warning: The file \"%s\" is larger than 4 GiB. It will not be visible in the ISO9660/Joliet file system."),
                            (*it_file)->file_name_.c_str());
                        progress.notify(ckcore::Progress::ckWARNING,StringTable::instance().get_string(StringTable::WARNING_SKIP4GFILEISO),
                            (*it_file)->file_name_.c_str());
                    }
                }

                // If imported, use the imported information.
                if ((*it_file)->file_flags_ & FileTreeNode::FLAG_IMPORTED)
                {
                    IsoImportData *import_node_ptr = static_cast<IsoImportData *>((*it_file)->data_ptr_);
                    if (import_node_ptr == NULL)
                    {
                        ckcore::tstringstream msg;
                        msg << ckT("The file \"") << (*it_file)->file_name_
                            << ckT("\" does not contain imported session data like advertised.");
                        throw ckcore::Exception2(msg.str());
                    }

                    (*it_file)->data_size_normal_ = import_node_ptr->extent_len_;
                    (*it_file)->data_size_joliet_ = import_node_ptr->extent_len_;

                    (*it_file)->data_pos_normal_ = import_node_ptr->extent_loc_;
                    (*it_file)->data_pos_joliet_ = import_node_ptr->extent_loc_;
                }
                else
                {
                    (*it_file)->data_size_normal_ = (*it_file)->file_size_;
                    (*it_file)->data_size_joliet_ = (*it_file)->file_size_;

                    (*it_file)->data_pos_normal_ = sec_offset;
                    (*it_file)->data_pos_joliet_ = sec_offset;

                    sec_offset += (*it_file)->data_size_normal_/ISO_SECTOR_SIZE;
                    if ((*it_file)->data_size_normal_ % ISO_SECTOR_SIZE != 0)
                        sec_offset++;

                    // Pad if necessary.
                    sec_offset += (*it_file)->data_pad_len_;
                }
            }
        }
    }

    void FileSystemWriter::calc_filesys_data(FileTree &file_tree,ckcore::Progress &progress,
                                             ckcore::tuint64 start_sec,ckcore::tuint64 &last_sec)
    {
        FileTreeNode *cur_node = file_tree.get_root();
        ckcore::tuint64 sec_offset = start_sec;

        std::vector<std::pair<FileTreeNode *,int> > dir_node_stack;
        calc_local_filesys_data(dir_node_stack,cur_node,2,sec_offset,progress);

        while (dir_node_stack.size() > 0)
        { 
            cur_node = dir_node_stack[dir_node_stack.size() - 1].first;
            int level = dir_node_stack[dir_node_stack.size() - 1].second;
            dir_node_stack.pop_back();

            calc_local_filesys_data(dir_node_stack,cur_node,level,sec_offset,progress);
        }

        last_sec = sec_offset;
    }

    void FileSystemWriter::write_file_node(SectorOutStream &out_stream,FileTreeNode *node,
                                           ckcore::Progresser &progresser)
    {
        // Make sure that the file stream is ready for reading. Please note that this
        // is the second place of try. The stream should already be open.
        if (!node->file_stream_.test() && node->file_stream_.open())
            throw FileOpenException(node->file_path_);

#ifdef _DEBUG
        node->data_pos_actual_ = out_stream.get_sector();
#endif

        // Validate the file size.
        if (ckcore::tuint64(node->file_stream_.size()) != node->file_size_)
        {
            if (fail_on_error_)
            {
                ckcore::tstringstream msg;
                msg << ckT("The file \"") << node->file_path_
                    << ckT("\" may have been modified during file system ")
                       ckT("creation, please close all applications accessing ")
                       ckT("the file and try again.");

                    throw ckcore::Exception2(msg.str());
            }
            else
            {
                progresser.notify(ckcore::Progress::ckWARNING,
                                  ckT("The file \"%s\" may have been modified ")
                                  ckT("during file system creation, please verify ")
                                  ckT("its integrity on the disc."),
                                  node->file_path_.c_str());
                log_.print_line(ckT("warning: conflicting file sizes in \"%s\"."),
                                node->file_path_.c_str());
            }
        }

        // Copy the file data into the disc file system.
        ckcore::CanexInStream in_stream(node->file_stream_,node->file_path_);
        ckcore::canexstream::copy(in_stream,out_stream,progresser,node->file_size_);

        // Pad the sector.
        if (out_stream.get_allocated() != 0)
            out_stream.pad_sector();
    }

    void FileSystemWriter::write_local_file_data(SectorOutStream &out_stream,
                                                 std::vector<std::pair<FileTreeNode *,int> > &dir_node_stack,
                                                 FileTreeNode *local_node,int level,
                                                 ckcore::Progresser &progresser)
    {
        std::vector<FileTreeNode *>::const_iterator it_file;
        for (it_file = local_node->children_.begin(); it_file !=
            local_node->children_.end(); it_file++)
        {
            // Check if we should abort.
            if (progresser.cancelled())
                return;

            if ((*it_file)->file_flags_ & FileTreeNode::FLAG_DIRECTORY)
            {
                // Validate directory level.
                if (level > file_sys_.get_max_dir_level())
                    continue;
                else
                    dir_node_stack.push_back(std::make_pair(*it_file,level + 1));
            }
            else if (!((*it_file)->file_flags_ & FileTreeNode::FLAG_IMPORTED))  // We don't have any data to write for imported files.
            {
                // Validate file size.
                if (file_sys_.is_iso() && !file_sys_.is_udf())
                {
                    if ((*it_file)->file_size_ > ISO_MAX_EXTENT_SIZE && !file_sys_.allows_fragmentation())
                        continue;
                }

                write_file_node(out_stream,*it_file,progresser);

                // The write operation might have been cancelled.
                if (progresser.cancelled())
                    return;

                // Pad if necessary.
                char tmp[1] = { 0 };
                for (unsigned int i = 0; i < (*it_file)->data_pad_len_; i++)
                {
                    for (unsigned int j = 0; j < ISO_SECTOR_SIZE; j++)
                        out_stream.write(tmp,1);
                }
            }
        }
    }

    void FileSystemWriter::write_file_data(SectorOutStream &out_stream,FileTree &file_tree,
                                           ckcore::Progresser &progresser)
    {
        FileTreeNode *cur_node = file_tree.get_root();

        std::vector<std::pair<FileTreeNode *,int> > dir_node_stack;
        write_local_file_data(out_stream,dir_node_stack,cur_node,2,progresser);
        if (progresser.cancelled())
            return;

        while (dir_node_stack.size() > 0)
        { 
            cur_node = dir_node_stack[dir_node_stack.size() - 1].first;
            int level = dir_node_stack[dir_node_stack.size() - 1].second;
            dir_node_stack.pop_back();

            write_local_file_data(out_stream,dir_node_stack,cur_node,level,progresser);
            if (progresser.cancelled())
                return;
        }
    }

    void FileSystemWriter::get_internal_path(FileTreeNode *child_node,ckcore::tstring &node_path,
                                             bool ext_path,bool joliet)
    {
        node_path = ckT("/");

        if (ext_path)
        {
            // Joliet or ISO9660?
            if (joliet)
            {
                const std::wstring::size_type joliet_len = child_node->file_name_joliet_.length();
#ifdef _UNICODE
                if (joliet_len >= 2 && child_node->file_name_joliet_[joliet_len - 2] == ';')
                    node_path.append(child_node->file_name_joliet_,0,joliet_len - 2);
                else
                    node_path.append(child_node->file_name_joliet_);
#else
                char ansi_name[JOLIET_MAX_NAMELEN_RELAXED + 1];
                ckcore::string::utf16_to_ansi(child_node->file_name_joliet_.c_str(),ansi_name,sizeof(ansi_name));

                if (joliet_len >= 2 && ansi_name[joliet_len - 2] == ';')
                    ansi_name[joliet_len - 2] = '\0';

                node_path.append(ansi_name);
#endif
            }
            else
            {
                const std::string::size_type iso_len = child_node->file_name_iso_.length();
#ifdef _UNICODE
                wchar_t utf_file_name[MAX_PATH];
                ckcore::string::ansi_to_utf16(child_node->file_name_iso_.c_str(),utf_file_name,
                                              sizeof(utf_file_name)/sizeof(wchar_t));

                if (iso_len >= 2 && utf_file_name[iso_len - 2] == ';')
                    utf_file_name[iso_len - 2] = '\0';

                node_path.append(utf_file_name);
#else
                if (iso_len >= 2 && child_node->file_name_iso_[iso_len - 2] == ';')
                    node_path.append(child_node->file_name_iso_,0,iso_len - 2);
                else
                    node_path.append(child_node->file_name_iso_);
#endif
            }
        }
        else
        {
            node_path.append(child_node->file_name_);
        }

        FileTreeNode *cur_node = child_node->parent();
        while (cur_node->parent() != NULL)
        {
            if (ext_path)
            {
                // Joliet or ISO9660?
                if (joliet)
                {
                    const std::wstring::size_type joliet_len = cur_node->file_name_joliet_.length();
    #ifdef _UNICODE
                    if (joliet_len >= 2 && cur_node->file_name_joliet_[joliet_len - 2] == ';')
                    {
                        std::wstring::iterator itEnd = cur_node->file_name_joliet_.end();
                        itEnd--;
                        itEnd--;

                        node_path.insert(node_path.begin(),cur_node->file_name_joliet_.begin(),itEnd);
                    }
                    else
                    {
                        node_path.insert(node_path.begin(),cur_node->file_name_joliet_.begin(),
                            cur_node->file_name_joliet_.end());
                    }
    #else
                    char ansi_name[JOLIET_MAX_NAMELEN_RELAXED + 1];
                    ckcore::string::utf16_to_ansi(cur_node->file_name_joliet_.c_str(),ansi_name,sizeof(ansi_name));

                    if (joliet_len >= 2 && ansi_name[joliet_len - 2] == ';')
                        ansi_name[joliet_len - 2] = '\0';

                    node_path.insert(0,ansi_name);
    #endif
                }
                else
                {
                    const std::string::size_type iso_len = cur_node->file_name_iso_.length();
    #ifdef _UNICODE
                    wchar_t utf_file_name[MAX_PATH];
                    ckcore::string::ansi_to_utf16(cur_node->file_name_iso_.c_str(),utf_file_name,
                                                  sizeof(utf_file_name)/sizeof(wchar_t));
                    node_path.insert(0,utf_file_name);

                    if (iso_len >= 2 && utf_file_name[iso_len - 2] == ';')
                        utf_file_name[iso_len - 2] = '\0';
    #else
                    if (iso_len >= 2 && cur_node->file_name_iso_[iso_len - 2] == ';')
                    {
                        std::string::iterator itEnd = cur_node->file_name_iso_.end();
                        itEnd--;
                        itEnd--;

                        node_path.insert(node_path.begin(),cur_node->file_name_iso_.begin(),itEnd);
                    }
                    else
                    {
                        node_path.insert(node_path.begin(),cur_node->file_name_iso_.begin(),
                            cur_node->file_name_iso_.end());
                    }
    #endif
                }
            }
            else
            {
                node_path.insert(node_path.begin(),cur_node->file_name_.begin(),
                    cur_node->file_name_.end());
            }

            node_path.insert(0,ckT("/"));

            cur_node = cur_node->parent();
        }
    }

    void FileSystemWriter::create_local_file_path_map(FileTreeNode *local_node,
                                                      std::vector<FileTreeNode *> &dir_node_stack,
                                                      std::map<ckcore::tstring,ckcore::tstring> &file_path_map,
                                                      bool joliet)
    {
        std::vector<FileTreeNode *>::const_iterator it_file;
        for (it_file = local_node->children_.begin(); it_file !=
            local_node->children_.end(); it_file++)
        {
            if ((*it_file)->file_flags_ & FileTreeNode::FLAG_DIRECTORY)
            {
                dir_node_stack.push_back(*it_file);
            }
            else
            {
                // Yeah, this is not very efficient. Both paths should be calculated togather.
                ckcore::tstring file_path;
                get_internal_path(*it_file,file_path,false,joliet);

                ckcore::tstring ExternalFilePath;
                get_internal_path(*it_file,ExternalFilePath,true,joliet);

                file_path_map[file_path] = ExternalFilePath;
            }
        }
    }

    void FileSystemWriter::create_file_path_map(FileTree &file_tree,
                                                std::map<ckcore::tstring,ckcore::tstring> &file_path_map,
                                                bool joliet)
    {
        FileTreeNode *cur_node = file_tree.get_root();

        std::vector<FileTreeNode *> dir_node_stack;
        create_local_file_path_map(cur_node,dir_node_stack,file_path_map,joliet);

        while (dir_node_stack.size() > 0)
        { 
            cur_node = dir_node_stack.back();
            dir_node_stack.pop_back();

            create_local_file_path_map(cur_node,dir_node_stack,file_path_map,joliet);
        }
    }

    int FileSystemWriter::write(ckcore::OutStream &out_stream,ckcore::Progress &progress,
                                ckcore::tuint32 sec_offset)
    {
        log_.print_line(ckT("FileSystemWriter::write"));
        log_.print_line(ckT("  sector offset: %u."),sec_offset);

        ckcore::BufferedOutStream out_buf_stream(out_stream);
        SectorOutStream out_sec_stream(out_buf_stream);

        // The first 16 sectors are reserved for system use (write 0s).
        char tmp[ISO_SECTOR_SIZE];
        memset(tmp,0,ISO_SECTOR_SIZE);
        for (unsigned int i = 0; i < 16; i++)
            out_stream.write(tmp,ISO_SECTOR_SIZE);

        progress.set_status(ckT("%s"),StringTable::instance().get_string(StringTable::STATUS_BUILDTREE));
        progress.set_marquee(true);

        try
        {
            // Create a file tree.
            if (!file_tree_.create_from_file_set(file_sys_.files()))
            {
                log_.print_line(ckT("error: failed to build file tree."));
                return RESULT_FAIL;
            }

            // Calculate padding if DVD-Video file system.
            if (file_sys_.is_dvdvideo())
            {
                DvdVideo dvd_video(log_);
                if (!dvd_video.calc_file_padding(file_tree_))
                {
                    progress.notify(ckcore::Progress::ckERROR,
                                    StringTable::instance().get_string(StringTable::ERROR_DVDVIDEO));

                    // Restore progress.
                    progress.set_marquee(false);
                    progress.set_progress(100);

                    log_.print_line(ckT("error: failed to calculate file padding for DVD-Video file system."));
                    return RESULT_FAIL;
                }

                dvd_video.print_file_padding(file_tree_);
            }

            bool is_iso = file_sys_.is_iso();
            bool is_udf = file_sys_.is_udf();
            bool is_joliet = file_sys_.is_joliet();

            SectorManager sec_manager(16 + sec_offset);
            IsoWriter iso_writer(log_,out_sec_stream,sec_manager,file_sys_,true,is_joliet);
            UdfWriter udf_writer(log_,out_sec_stream,sec_manager,file_sys_,true);

            // FIXME: Put failure messages to Progress.
            if (is_iso)
                iso_writer.alloc_header();

            if (is_udf)
                udf_writer.alloc_header();

            if (is_udf)
                udf_writer.alloc_partition(file_tree_);

            IsoPathTable pt_iso,pt_jol;
            if (is_iso)
            {
                // Make proper names.
                iso_writer.calc_names(file_tree_);

                // Populate and sort path tables.
                iso_path_table_populate(pt_iso,file_tree_,file_sys_,progress);
                iso_path_table_sort(pt_iso,false,file_sys_.is_dvdvideo());

                if (file_sys_.is_joliet())
                {
                    iso_path_table_populate(pt_jol,file_tree_,file_sys_,progress);
                    iso_path_table_sort(pt_jol,true,file_sys_.is_dvdvideo());
                }

                iso_writer.alloc_path_tables(pt_iso,pt_jol,progress);
                iso_writer.alloc_dir_entries(file_tree_);
            }

            // Allocate file data.
            ckcore::tuint64 first_data_sec = sec_manager.get_next_free();
            ckcore::tuint64 last_data_sec = 0;

            calc_filesys_data(file_tree_,progress,first_data_sec,last_data_sec);

            sec_manager.alloc_data_sectors(last_data_sec - first_data_sec);

            int res = RESULT_FAIL;

            if (is_iso)
                iso_writer.write_header(file_sys_.files(),file_tree_);

            if (is_udf)
                udf_writer.write_header();

            if (is_udf)
                udf_writer.write_partition(file_tree_);

            // FIXME: Add progress for this.
            if (is_iso)
            {
                iso_writer.write_path_tables(pt_iso,pt_jol,file_tree_,progress);

                res = iso_writer.write_dir_entries(file_tree_,progress);
                if (res != RESULT_OK)
                {
                    // Restore progress.
                    progress.set_marquee(false);
                    progress.set_progress(100);

                    return res;
                }
            }

            progress.set_status(ckT("%s"),StringTable::instance().get_string(StringTable::STATUS_WRITEDATA));
            progress.set_marquee(false);

            // To help keep track of the progress.
            ckcore::Progresser progresser(progress,sec_manager.get_data_length() * ISO_SECTOR_SIZE);
            write_file_data(out_sec_stream,file_tree_,progresser);
            if (progresser.cancelled())
                return RESULT_CANCEL;

            if (is_udf)
                udf_writer.write_tail();

            out_buf_stream.flush();
#ifdef _DEBUG
            file_tree_.print_tree();
#endif
        }
        catch (FileOpenException &e)
        {
            progress.notify(ckcore::Progress::ckERROR,
                            StringTable::instance().get_string(StringTable::ERROR_OPENREAD),
                            e.file_path().c_str());

            // Restore progress.
            progress.set_marquee(false);
            progress.set_progress(100);

            // Write message to log file.
            log_.print_line(ckT("Error: %s"),get_except_msg(e).c_str());
            return RESULT_FAIL;
        }
        catch (const std::exception &e)
        {
            progress.notify(ckcore::Progress::ckERROR,ckT("%s"),ckcore::get_except_msg(e).c_str());

            // Restore progress.
            progress.set_marquee(false);
            progress.set_progress(100);

            // Write message to log file.
            log_.print_line(ckT("Error: %s"),ckcore::get_except_msg(e).c_str());
            return RESULT_FAIL;
        }

        return RESULT_OK;
    }

    int FileSystemWriter::file_path_map(std::map<ckcore::tstring,ckcore::tstring> &file_path_map)
    {
        create_file_path_map(file_tree_,file_path_map,file_sys_.is_joliet());
        return RESULT_OK;
    }
};
