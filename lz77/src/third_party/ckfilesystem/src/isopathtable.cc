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

#include "ckfilesystem/isopathtable.hh"  // The include file for this module comes first.

#include <string.h>
#include <algorithm>
#include "ckfilesystem/stringtable.hh"

namespace ckfilesystem
{
    int level(const FileTreeNode *node)
    {
        int item_level = 0;

        FileTreeNode *cur_node = const_cast<FileTreeNode *>(node);
        while (cur_node != NULL)
        {
            item_level++;
            cur_node = cur_node->parent();
        }

        return item_level;
    }

    void iso_path_table_populate_locally(std::vector<FileTreeNode *> &node_stack,
                                         IsoPathTable &pt,
                                         FileTreeNode *node)
    {
        std::vector<FileTreeNode *>::const_iterator it;
        for (it = node->children_.begin(); it !=
            node->children_.end(); it++)
        {
            if ((*it)->file_flags_ & FileTreeNode::FLAG_DIRECTORY)
                node_stack.push_back(*it);
        }
    }

    void iso_path_table_populate(IsoPathTable &pt, FileTree &tree,
                                 FileSystem &file_sys,
                                 ckcore::Progress &progress)
    {
        std::vector<FileTreeNode *> node_stack;
        iso_path_table_populate_locally(node_stack, pt, tree.get_root());

        // Set to true of we have found that the directory structure is to
        // deep. This variable is needed so that the warning message will
        // only be printed once.
        bool found_deep = false;

        while (node_stack.size() > 0)
        {
            FileTreeNode *node = node_stack.back();
            node_stack.pop_back();

            // Ignore all nodes deeper than the maximum level.
            int node_level = level(node);
            if (node_level > file_sys.get_max_dir_level())
            {
                // Print the message only once.
                if (!found_deep)
                {
                    //log_.print_line(ckT("  Warning: The directory structure is deeper than %d levels. Deep files and folders will be ignored."),
                    //             file_sys_.iso_.get_max_dir_level());
                    progress.notify(ckcore::Progress::ckWARNING,
                                    StringTable::instance().get_string(StringTable::WARNING_FSDIRLEVEL),
                                    file_sys.get_max_dir_level());
                    found_deep = true;
                }

                //log_.print_line(ckT("  Skipping: %s."),cur_node->file_path_.c_str());
                progress.notify(ckcore::Progress::ckWARNING,
                                StringTable::instance().get_string(StringTable::WARNING_SKIPFILE),
                                node->file_path_.c_str());
                continue;
            }

            pt.push_back(std::make_pair(node, 0));

            iso_path_table_populate_locally(node_stack, pt, node);
        }
    }

    /**
     * Returns a weight of the specified file name, a lighter file should
     * be placed heigher in the directory hierarchy.
     */
    ckcore::tuint32 entry_weight(const IsoPathTableEntry &entry)
    {
        ckcore::tuint32 weight = 0xffffffff;

        // Calculate path.
        std::vector<FileTreeNode *> node_stack;

        FileTreeNode *cur_node = const_cast<FileTreeNode *>(entry.first);
        while (cur_node != NULL)
        {
            node_stack.push_back(cur_node);
            cur_node = cur_node->parent();
        }

        ckcore::tstringstream path;

        std::vector<FileTreeNode *>::const_iterator it;
        for (it = node_stack.begin(); it != node_stack.end(); it++)
            path << ckT("/") << (*it)->file_name_;

        const ckcore::tstring file_path = path.str();

        // Quick test for optimization.
        if (file_path[1] == 'V')
        {
            if (!ckcore::string::astrcmp(file_path.c_str(),ckT("/VIDEO_TS")))   // The VIDEO_TS folder should be first.
                weight = 0;
            /*else if (!ckcore::string::astrncmp(file_path.c_str(),ckT("/VIDEO_TS/"),10))
            {
                const ckcore::tchar *file_name = file_path.c_str() + 10;

                if (!ckcore::string::astrncmp(file_name,ckT("VIDEO_TS"),8))
                {
                    weight -= 0x80000000;

                    const ckcore::tchar *file_ext = file_name + 9;
                    if (!ckcore::string::astrcmp(file_ext,ckT("IFO")))
                        weight -= 3;
                    else if (!ckcore::string::astrcmp(file_ext,ckT("VOB")))
                        weight -= 2;
                    else if (!ckcore::string::astrcmp(file_ext,ckT("BUP")))
                        weight -= 1;
                }
                else if (!ckcore::string::astrncmp(file_name,ckT("VTS_"),4))
                {
                    weight -= 0x40000000;

                    // Just a safety measure.
                    if (ckcore::string::astrlen(file_name) < 64)
                    {
                        ckcore::tchar file_ext[64+1];
                        memset( file_ext, 0, sizeof(file_ext) );  // scanf %c does not append the null-terminator
                        ckcore::tuint32 num = 0,sub_num = 0;

                        if (asscanf(file_name,ckT("VTS_%u_%u.%64c"),&num,&sub_num,file_ext) == 3)
                        {
                            // The first number is worth the most, the lower the lighter.
                            weight -= 0xffffff - (num << 8);

                            if (!ckcore::string::astrcmp(file_ext,ckT("IFO")))
                            {
                                weight -= 0xff;
                            }
                            else if (!ckcore::string::astrcmp(file_ext,ckT("VOB")))
                            {
                                weight -= 0x0f - sub_num;
                            }
                            else if (!ckcore::string::astrcmp(file_ext,ckT("BUP")))
                            {
                                weight -= 1;
                            }
                        }
                    }
                }
            }*/
        }

        return weight;
    }

    bool level_predicate(const IsoPathTableEntry &e1,
                         const IsoPathTableEntry &e2)
    {
        return level(e1.first) < level(e2.first);
    }

    bool main_predicate_iso(const IsoPathTableEntry &e1,
                            const IsoPathTableEntry &e2)
    {
        if (e1.second < e2.second)
            return true;
        else if (e1.second == e2.second)
            return strcmp(e1.first->file_name_iso_.c_str(),
                          e2.first->file_name_iso_.c_str()) < 0;
        else
            return false;
    }

    bool main_predicate_iso_dvd(const IsoPathTableEntry &e1,
                                const IsoPathTableEntry &e2)
    {
        ckcore::tuint32 weight1 = entry_weight(e1);
        ckcore::tuint32 weight2 = entry_weight(e2);

        if (weight1 != weight2)
        {
            if (weight1 < weight2)
                return true;
            else
                return false;
        }

        if (e1.second < e2.second)
            return true;
        else if (e1.second == e2.second)
            return strcmp(e1.first->file_name_iso_.c_str(),
                          e2.first->file_name_iso_.c_str()) < 0;
        else
            return false;
    }

    bool main_predicate_jol(const IsoPathTableEntry &e1,
                            const IsoPathTableEntry &e2)
    {
        if (e1.second < e2.second)
            return true;
        else if (e1.second == e2.second)
            return wcscmp(e1.first->file_name_joliet_.c_str(),
                          e2.first->file_name_joliet_.c_str()) < 0;
        else
            return false;
    }

    bool main_predicate_jol_dvd(const IsoPathTableEntry &e1,
                                const IsoPathTableEntry &e2)
    {
        ckcore::tuint32 weight1 = entry_weight(e1);
        ckcore::tuint32 weight2 = entry_weight(e2);

        if (weight1 != weight2)
        {
            if (weight1 < weight2)
                return true;
            else
                return false;
        }

        if (e1.second < e2.second)
            return true;
        else if (e1.second == e2.second)
            return wcscmp(e1.first->file_name_joliet_.c_str(),
                          e2.first->file_name_joliet_.c_str()) < 0;
        else
            return false;
    }

    ckcore::tuint16 find_parent(FileTreeNode *node, IsoPathTable &pt)
    {
        if (pt.size() > 0xffff)
            throw ckcore::Exception2(ckT("Too many directories in ISO9660 file system. ")
                                     ckT("Directory indentifier exceeded 0xffff."));

        for (ckcore::tuint16 i = 0; i < static_cast<ckcore::tuint16>(pt.size()); i++)
            if (pt[i].first == node)
                return i + 2;   // Root is 1 so the first identifier will be 2.

        throw ckcore::Exception2(ckT("Internal error: Unable to locate parent ")
                                 ckT("directory in path table."));
    }

    void calc_ids(IsoPathTable::iterator &begin, IsoPathTable::iterator &end)
    {
        IsoPathTable::iterator it;
        for (it = begin; it != end; it++)
            it->second = 1; // The parent is obviously the root.
    }

    void calc_ids(IsoPathTable::iterator &begin, IsoPathTable::iterator &end,
                  IsoPathTable &pt)
    {
        IsoPathTable::iterator it;
        for (it = begin; it != end; it++)
            it->second = find_parent(it->first->parent(),pt);
    }

    void iso_path_table_sort(IsoPathTable &pt, bool joliet, bool dvdvideo)
    {
        // First, sort everything by level.
        std::sort(pt.begin(), pt.end(), level_predicate);

        // Locate all levels.
        std::vector<std::pair<IsoPathTable::iterator,
                              IsoPathTable::iterator> > levels;

        IsoPathTable::iterator it_begin = pt.begin();
        if (it_begin != pt.end())
        {
            int prv_level = level(it_begin->first);

            IsoPathTable::iterator it;
            for (it = pt.begin(); it != pt.end(); it++)
            {
                int new_level = level(it->first);
                if (new_level != prv_level)
                {
                    levels.push_back(std::make_pair(it_begin, it));

                    it_begin = it;

                    prv_level = new_level;
                }
            }

            // Add the last level.
            levels.push_back(std::make_pair(it_begin, pt.end()));
        }

        // Select sort predicate.
        bool (*sort_predicate)(const IsoPathTableEntry &,
                               const IsoPathTableEntry &) = NULL;

        if (joliet)
            sort_predicate = dvdvideo ? main_predicate_jol_dvd : main_predicate_jol;
        else
            sort_predicate = dvdvideo ? main_predicate_iso_dvd : main_predicate_iso;

        for (size_t i = 0; i < levels.size(); i++)
        {
            // First, find all parent identifiers.
            if (i == 0)
                calc_ids(levels[i].first, levels[i].second);
            else
                calc_ids(levels[i].first, levels[i].second, pt);

            // Now we're ready to sort.
            std::sort(levels[i].first, levels[i].second, sort_predicate);
        }
    }
}
