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

#include <cassert>
#include "ckfilesystem/filetree.hh"

namespace ckfilesystem
{
    FileTree::FileTree(ckcore::Log &log) :
        log_(log),root_node_(NULL),dir_count_(0),file_count_(0)
    {
    }

    FileTree::~FileTree()
    {
        if (root_node_ != NULL)
        {
            delete root_node_;
            root_node_ = NULL;
        }
    }

    FileTreeNode *FileTree::get_root()
    {
        return root_node_;
    }

    /**
     * Find the children inside a parent node given the name of the child.
     * @param [in] parent_node The parent node.
     * @param [in] file_name The name of the child.
     * @return If successful a pointer to the child now, if not NULL is
     *         returned.
     */
    FileTreeNode *FileTree::get_child_from_file_name(FileTreeNode *parent_node,const ckcore::tchar *file_name)
    {
        std::vector<FileTreeNode *>::const_iterator it;
        for (it = parent_node->children_.begin(); it !=
            parent_node->children_.end(); it++)
        {
            if (!ckcore::string::astrcmp(file_name,(*it)->file_name_.c_str()))
                return *it;
        }

        return NULL;
    }

    bool FileTree::add_file_from_path(const FileDescriptor &file)
    {
        size_t dir_path_len = file.internal_path_.length(),prev_delim = 0,pos;
        ckcore::tstring cur_dir_name;
        FileTreeNode *cur_node = root_node_;

        for (pos = 0; pos < dir_path_len; pos++)
        {
            if (file.internal_path_.c_str()[pos] == '/')
            {
                if (pos > (prev_delim + 1))
                {
                    // Obtain the name of the current directory.
                    cur_dir_name.erase();
                    for (size_t j = prev_delim + 1; j < pos; j++)
                        cur_dir_name.push_back(file.internal_path_.c_str()[j]);

                    cur_node = get_child_from_file_name(cur_node,cur_dir_name.c_str());
                    if (cur_node == NULL)
                    {
                        log_.print_line(ckT("  Error: Unable to find child node \"%s\" in path \"%s\"."),
                            cur_dir_name.c_str(),file.internal_path_.c_str());
                        return false;
                    }
                }

                prev_delim = pos;
            }
        }

        // We now have our parent.
        const ckcore::tchar *file_name = file.internal_path_.c_str() + prev_delim + 1;

        // Check if imported.
        unsigned char import_flag = 0;
        void *import_data_ptr = NULL;
        if (file.flags_ & FileDescriptor::FLAG_IMPORTED)
        {
            import_flag = FileTreeNode::FLAG_IMPORTED;
            import_data_ptr = file.data_ptr_;
        }

        if (file.flags_ & FileDescriptor::FLAG_DIRECTORY)
        {
            cur_node->children_.push_back(new FileTreeNode(cur_node,file_name,
                file.external_path_.c_str(),true,0,FileTreeNode::FLAG_DIRECTORY | import_flag,
                import_data_ptr));

            dir_count_++;
        }
        else
        {
            cur_node->children_.push_back(new FileTreeNode(cur_node,file_name,
                file.external_path_.c_str(),true,0,import_flag,import_data_ptr));

            file_count_++;
        }

        return true;
    }

    bool FileTree::create_from_file_set(const FileSet &files)
    {
        if (root_node_ != NULL)
            delete root_node_;

        root_node_ = new FileTreeNode(NULL,ckT(""),ckT(""),true,0,
                                      FileTreeNode::FLAG_DIRECTORY);

        FileSet::const_iterator it;
        for (it = files.begin(); it != files.end(); it++)
        {
            ckfilesystem::FileDescriptor * fd = *it;
            if (!add_file_from_path(*fd))
                return false;
        }

        return true;
    }

    // CONTINUE: Investigate how this can be made to use exceptions.
    FileTreeNode *FileTree::get_node_from_path(const ckcore::tchar *internal_path)
    {
        size_t dir_path_len = ckcore::string::astrlen(internal_path),prev_delim = 0,pos;
        ckcore::tstring cur_dir_name;
        FileTreeNode *cur_node = root_node_;

        for (pos = 0; pos < dir_path_len; pos++)
        {
            if (internal_path[pos] == '/')
            {
                if (pos > (prev_delim + 1))
                {
                    // Obtain the name of the current directory.
                    cur_dir_name.erase();
                    for (size_t j = prev_delim + 1; j < pos; j++)
                        cur_dir_name.push_back(internal_path[j]);

                    cur_node = get_child_from_file_name(cur_node,cur_dir_name.c_str());
                    if (cur_node == NULL)
                    {
                        log_.print_line(ckT("  Error: Unable to find child node \"%s\"."),cur_dir_name.c_str());
                        return NULL;
                    }
                }

                prev_delim = pos;
            }
        }

        // We now have our parent.
        const ckcore::tchar *file_name = internal_path + prev_delim + 1;

        return get_child_from_file_name(cur_node,file_name);
    }

    /**
        @eturn the number of files in the tree, fragmented files are counted once.
    */
    ckcore::tuint32 FileTree::get_dir_count()
    {
        return dir_count_;
    }

    /**
        @return the number of directories in the tree, the root is not included.
    */
    ckcore::tuint32 FileTree::get_file_count()
    {
        return file_count_;
    }

#ifdef _DEBUG
    void FileTree::print_local_tree(std::vector<std::pair<FileTreeNode *,int> > &dir_node_stack,
                                    FileTreeNode *local_node,int indent)
    {
        std::vector<FileTreeNode *>::const_iterator it;
        for (it = local_node->children_.begin(); it !=
            local_node->children_.end(); it++)
        {
            if ((*it)->file_flags_ & FileTreeNode::FLAG_DIRECTORY)
            {
                dir_node_stack.push_back(std::make_pair(*it,indent));
            }
            else
            {
                for (int i = 0; i < indent; i++)
                    log_.print(ckT(" "));

                log_.print(ckT("<f>"));
                log_.print((*it)->file_name_.c_str());
#ifdef _WINDOWS
                log_.print_line(ckT(" (%I64u:%I64u:%I64u,%I64u:%I64u,%I64u:%I64u)"),(*it)->data_pos_normal_,
#else
                log_.print_line(ckT(" (%llu:%llu:%llu,%llu:%llu,%llu:%llu)"),(*it)->data_pos_normal_,
#endif
                    (*it)->data_size_normal_,(*it)->data_pos_joliet_,(*it)->data_pos_actual_,
                    (*it)->data_size_joliet_,(*it)->udf_size_,(*it)->udf_size_tot_);
            }
        }
    }

    void FileTree::print_tree()
    {
        if (root_node_ == NULL)
            return;

        FileTreeNode *cur_node = root_node_;
        int indent = 0;

        log_.print_line(ckT("FileTree::print_tree"));
#ifdef _WINDOWS
        log_.print_line(ckT("  <root> (%I64u:%I64u,%I64u:%I64u,%I64u:%I64u)"),cur_node->data_pos_normal_,
#else
        log_.print_line(ckT("  <root> (%llu:%llu,%llu:%llu,%llu:%llu)"),cur_node->data_pos_normal_,
#endif
        cur_node->data_size_normal_,cur_node->data_pos_joliet_,
        cur_node->data_size_joliet_,cur_node->udf_size_,cur_node->udf_size_tot_);

        std::vector<std::pair<FileTreeNode *,int> > dir_node_stack;
        print_local_tree(dir_node_stack,cur_node,4);

        while (dir_node_stack.size() > 0)
        { 
            cur_node = dir_node_stack[dir_node_stack.size() - 1].first;
            indent = dir_node_stack[dir_node_stack.size() - 1].second;

            dir_node_stack.pop_back();

            // Print the directory name.
            for (int i = 0; i < indent; i++)
                log_.print(ckT(" "));

            log_.print(ckT("<d>"));
            log_.print(cur_node->file_name_.c_str());
#ifdef _WINDOWS
            log_.print_line(ckT(" (%I64u:%I64u:%I64u,%I64u:%I64u,%I64u:%I64u)"),cur_node->data_pos_normal_,
#else
            log_.print_line(ckT(" (%llu:%llu:%llu,%llu:%llu,%llu:%llu)"),cur_node->data_pos_normal_,
#endif
                cur_node->data_size_normal_,cur_node->data_pos_joliet_,cur_node->data_pos_actual_,
                cur_node->data_size_joliet_,cur_node->udf_size_,cur_node->udf_size_tot_);

            print_local_tree(dir_node_stack,cur_node,indent + 2);
        }
    }
#endif

    ckcore::tstring FileTree::get_path(const FileTreeNode *node, FileTreeNameType name_type) const
    {
        assert(node);

        ckcore::tstring path;

        node->bottom_up(
            [&](const FileTreeNode &node)
            {
                // Skip the root node.
                if (node.parent() == NULL)
                    return;

                switch (name_type)
                {
                    case NAME_ORIGINAL:
                        path.insert(0, ckcore::tstring(ckT("/")) + node.file_name_);
                        break;
                    case NAME_ISO:
                        path.insert(0, ckcore::tstring(ckT("/")) + ckcore::string::to_auto(node.file_name_iso_));
                        break;
                    case NAME_JOLIET:
                        path.insert(0, ckcore::tstring(ckT("/")) + node.file_name_joliet_);
                        break;
                    default:
                        assert(false);
                        break;
                }
            });

        return path;
    }

    std::vector<ckcore::tstring> FileTree::serialize_local_tree(std::vector<FileTreeNode *> &dir_node_stack,
                                                                FileTreeNode *local_node, FileTreeNameType name_type) const
    {
        std::vector<ckcore::tstring> res;

        std::vector<FileTreeNode *>::const_iterator it;
        for (it = local_node->children_.begin(); it !=
            local_node->children_.end(); it++)
        {
            if ((*it)->file_flags_ & FileTreeNode::FLAG_DIRECTORY)
                dir_node_stack.push_back(*it);
            else
                res.push_back(get_path(*it, name_type));
        }

        return res;
    }

    std::vector<ckcore::tstring> FileTree::serialize(FileTreeNameType name_type) const
    {
        std::vector<ckcore::tstring> res, tmp;

        if (root_node_ == NULL)
            return res;

        FileTreeNode *cur_node = root_node_;

        std::vector<FileTreeNode *> dir_node_stack;

        tmp = serialize_local_tree(dir_node_stack, cur_node, name_type);
        res.insert(res.end(), tmp.begin(), tmp.end());

        while (dir_node_stack.size() > 0)
        {
            cur_node = dir_node_stack[dir_node_stack.size() - 1];
            dir_node_stack.pop_back();

            res.push_back(get_path(cur_node, name_type));
            tmp = serialize_local_tree(dir_node_stack, cur_node, name_type);
            res.insert(res.end(), tmp.begin(), tmp.end());
        }

        return res;
    }
};
