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

#include <algorithm>
#include <ckcore/convert.hh>
#include "ckfilesystem/dvdvideo.hh"
#include "ckfilesystem/iforeader.hh"

namespace ckfilesystem
{
    DvdVideo::DvdVideo(ckcore::Log &log) : log_(log)
    {
    }

    DvdVideo::~DvdVideo()
    {
    }

    ckcore::tuint64 DvdVideo::size_to_dvd_len(ckcore::tuint64 file_size)
    {
        return file_size / DVDVIDEO_BLOCK_SIZE;
    }

    FileTreeNode *DvdVideo::find_video_node(FileTree &file_tree,FileSetType type,ckcore::tuint32 number)
    {
        ckcore::tstring internal_path = ckT("/VIDEO_TS/");

        switch (type)
        {
            case FST_INFO:
                if (number == 0)
                {
                    internal_path.append(ckT("VIDEO_TS.IFO"));
                }
                else
                {
                    ckcore::tchar file_name[13];
                    ckcore::convert::sprintf(file_name,sizeof(file_name),ckT("VTS_%02d_0.IFO"),number);
                    internal_path.append(file_name);
                }
                break;

            case FST_BACKUP:
                if (number == 0)
                {
                    internal_path.append(ckT("VIDEO_TS.BUP"));
                }
                else
                {
                    ckcore::tchar file_name[13];
                    ckcore::convert::sprintf(file_name,sizeof(file_name),ckT("VTS_%02d_0.BUP"),number);
                    internal_path.append(file_name);
                }
                break;

            case FST_MENU:
                if (number == 0)
                {
                    internal_path.append(ckT("VIDEO_TS.VOB"));
                }
                else
                {
                    ckcore::tchar file_name[13];
                    ckcore::convert::sprintf(file_name,sizeof(file_name),ckT("VTS_%02d_0.VOB"),number);
                    internal_path.append(file_name);
                }
                break;

            case FST_TITLE:
                {
                    if (number == 0)
                        return NULL;

                    FileTreeNode *last_node = NULL;

                    // We find the last title node. There may be many of them.
                    ckcore::tchar file_name[13];
                    for (unsigned int i = 0; i < 9; i++)
                    {
                        file_name[0] = '\0';
                        ckcore::convert::sprintf(file_name,sizeof(file_name),ckT("VTS_%02d_%d.VOB"),number,i + 1);
                        internal_path.append(file_name);

                        FileTreeNode *node = file_tree.get_node_from_path(internal_path.c_str());
                        if (node == NULL)
                            break;

                        last_node = node;

                        // Restore the full path variable.
                        internal_path = ckT("/VIDEO_TS/");
                    }

                    // Since we're dealing with multiple files we return immediately.
                    return last_node;
                }

            default:
                return NULL;
        }

        return file_tree.get_node_from_path(internal_path.c_str());
    }

    bool DvdVideo::get_total_titles_size(ckcore::tstring &file_path,FileSetType type,
                                         ckcore::tuint32 number,ckcore::tuint64 &file_size)
    {
        ckcore::tstring full_path = file_path;

        if (number == 0)
            return false;

        file_size = 0;

        ckcore::tchar file_name[13];
        for (unsigned int i = 0; i < 9; i++)
        {
            file_name[0] = '\0';
            ckcore::convert::sprintf(file_name,sizeof(file_name),ckT("VTS_%02d_%d.VOB"),number,i + 1);
            full_path.append(file_name);

            if (!ckcore::File::exist(full_path.c_str()))
                break;

            file_size += ckcore::File::size(full_path.c_str());

            // Restore the full path variable.
            full_path = file_path;
        }

        return true;
    }

    bool DvdVideo::read_file_set_info_root(FileTree &file_tree,IfoVmgData &vmg_data,
                                           std::vector<ckcore::tuint32> &ts_sectors)
    {
        ckcore::tuint64 menu_size = 0,info_size = 0;

        FileTreeNode *info_node = find_video_node(file_tree,FST_INFO,0);
        if (info_node != NULL)
            info_size = info_node->file_size_;

        FileTreeNode *menu_node = find_video_node(file_tree,FST_MENU,0);
        if (menu_node != NULL)
            menu_size = menu_node->file_size_;

        FileTreeNode *bkup_node = find_video_node(file_tree,FST_BACKUP,0);

        // Verify the information.
        if ((vmg_data.last_vmg_sec_ + 1) < (size_to_dvd_len(info_size) << 1))
        {
            log_.print_line(ckT("error: invalid VIDEO_TS.IFO file size."));
            return false;
        }

        // Find the actual size of .IFO.
        ckcore::tuint64 info_len = 0;
        if (menu_node == NULL)
        {
            if ((vmg_data.last_vmg_sec_ + 1) > (size_to_dvd_len(info_size) << 1))
                info_len = vmg_data.last_vmg_sec_ - size_to_dvd_len(info_size) + 1;
            else
                info_len = vmg_data.last_vmg_ifo_sec_ + 1;
        }
        else
        {
            if ((vmg_data.last_vmg_ifo_sec_ + 1) < vmg_data.vmg_menu_vob_sec_)
                info_len = vmg_data.vmg_menu_vob_sec_;
            else
                info_len = vmg_data.last_vmg_ifo_sec_ + 1;
        }

        if (info_len > 0xffffffff)
        {
#ifdef _WINDOWS
            log_.print_line(ckT("error: VIDEO_TS.IFO is larger than 4 million blocks (%I64u blocks)."),info_len);
#else
            log_.print_line(ckT("error: VIDEO_TS.IFO is larger than 4 million blocks (%llu blocks)."),info_len);
#endif 
            return false;
        }

        if (info_node != NULL)
            info_node->data_pad_len_ = (ckcore::tuint32)info_len - (ckcore::tuint32)size_to_dvd_len(info_size);

        // Find the actual size of .VOB.
        ckcore::tuint64 menu_len = 0;
        if (menu_node != NULL)
        {
            menu_len = vmg_data.last_vmg_sec_ - info_len - size_to_dvd_len(info_size) + 1;

            if (menu_len > 0xffffffff)
            {
#ifdef _WINDOWS
                log_.print_line(ckT("error: VIDEO_TS.VOB is larger than 4 million blocks (%I64u blocks)."),menu_len);
#else
                log_.print_line(ckT("error: VIDEO_TS.VOB is larger than 4 million blocks (%lld blocks)."),menu_len);
#endif
                return false;
            }

            menu_node->data_pad_len_ = (ckcore::tuint32)menu_len - (ckcore::tuint32)size_to_dvd_len(menu_size);
        }

        // Find the actual size of .BUP.
        ckcore::tuint64 bkup_len = 0;
        if (ts_sectors.size() > 0)
            bkup_len = *ts_sectors.begin() - menu_len - info_len;
        else            
            bkup_len = vmg_data.last_vmg_sec_ + 1 - menu_len - info_len;    // If no title sets are used.

        if (bkup_len > 0xffffffff)
        {
#ifdef _WINDOWS
            log_.print_line(ckT("error: VIDEO_TS.BUP is larger than 4 million blocks (%I64u blocks)."),bkup_len);
#else
            log_.print_line(ckT("error: VIDEO_TS.BUP is larger than 4 million blocks (%llu blocks)."),bkup_len);
#endif
            return false;
        }

        if (bkup_node != NULL)
            bkup_node->data_pad_len_ = (ckcore::tuint32)bkup_len - (ckcore::tuint32)size_to_dvd_len(info_size);

        return true;
    }

    bool DvdVideo::read_file_set_info(FileTree &file_tree,std::vector<ckcore::tuint32> &ts_sectors)
    {
        ckcore::tuint32 counter = 1;

        std::vector<ckcore::tuint32>::const_iterator it_ts;
        for (it_ts = ts_sectors.begin(); it_ts != ts_sectors.end(); it_ts++)
        {
            FileTreeNode *info_node = find_video_node(file_tree,FST_INFO,counter);
            if (info_node == NULL)
            {
                log_.print_line(ckT("error: unable to find IFO file in file tree."));
                return false;
            }

            IfoReader ifo_reader(info_node->file_path_.c_str());
            if (!ifo_reader.open())
            {
                log_.print_line(ckT("error: unable to open and identify %s."),info_node->file_name_.c_str());
                return false;
            }

            if (ifo_reader.get_type() != IfoReader::IT_VTS)
            {
                log_.print_line(ckT("error: %s is not of VTS format."),info_node->file_name_.c_str());
                return false;
            }

            IfoVtsData vts_data;
            if (!ifo_reader.read_vts(vts_data))
            {
                log_.print_line(ckT("error: unable to read VTS data from %s."),info_node->file_name_.c_str());
                return false;
            }

            // Test if VTS_XX_0.VOB is present.
            ckcore::tuint64 menu_size = 0;
            FileTreeNode *menu_node = find_video_node(file_tree,FST_MENU,counter);
            if (menu_node != NULL)
                menu_size = menu_node->file_size_;

            // Test if VTS_XX_X.VOB are present.
            ckcore::tuint64 title_size = 0;

            ckcore::tstring file_path = info_node->file_path_;
            file_path.resize(file_path.find_last_of('/') + 1);

            bool title = get_total_titles_size(file_path,FST_TITLE,counter,title_size);

            // Test if VTS_XX_0.IFO are present.
            ckcore::tuint64 info_size = info_node->file_size_;

            // Check that the title will fit in the space given by the IFO file.
            if ((vts_data.last_vts_sec_ + 1) < (size_to_dvd_len(info_size) << 1))
            {
                log_.print_line(ckT("error: invalid size of %s."),info_node->file_name_.c_str());
                return false;
            }
            else if (title && menu_node != NULL && (vts_data.last_vts_sec_ + 1 < (size_to_dvd_len(info_size) << 1) +
                size_to_dvd_len(title_size) +  size_to_dvd_len(menu_size)))
            {
                log_.print_line(ckT("error: either IFO or menu VOB related to %s is of incorrect size. (1)"),
                    info_node->file_name_.c_str());
                return false;
            }
            else if (title && menu_node == NULL && (vts_data.last_vts_sec_ + 1 < (size_to_dvd_len(info_size) << 1) +
                size_to_dvd_len(title_size)))
            {
                log_.print_line(ckT("error: either IFO or menu VOB related to %s is of incorrect size. (2)"),
                    info_node->file_name_.c_str());
                return false;
            }
            else if (!title && menu_node != NULL && (vts_data.last_vts_sec_ + 1 < (size_to_dvd_len(info_size) << 1) +
                    size_to_dvd_len(menu_size)))
            {
                log_.print_line(ckT("error: either IFO or menu VOB related to %s is of incorrect size. (3)"),
                    info_node->file_name_.c_str());
                return false;
            }

            // Find the actual size of VTS_XX_0.IFO.
            ckcore::tuint64 info_len = 0;
            if (!title && menu_node == NULL)
            {
                info_len = vts_data.last_vts_sec_ - size_to_dvd_len(info_size) + 1;
            }
            else if (!title)
            {
                info_len = vts_data.vts_vob_sec_;
            }
            else
            {
                if (vts_data.last_vts_ifo_sec_ + 1 < vts_data.vts_menu_vob_sec_)
                    info_len = vts_data.vts_menu_vob_sec_;
                else
                    info_len = vts_data.last_vts_ifo_sec_ + 1;
            }

            if (info_len > 0xffffffff)
            {
                log_.print_line(ckT("error: IFO file larger than 4 million blocks."));
                return false;
            }

            info_node->data_pad_len_ = (ckcore::tuint32)info_len - (ckcore::tuint32)size_to_dvd_len(info_size);

            // Find the actual size of VTS_XX_0.VOB.
            ckcore::tuint64 menu_len = 0;
            if (menu_node != NULL)
            {
                if (title && (vts_data.vts_vob_sec_ - vts_data.vts_menu_vob_sec_ > size_to_dvd_len(menu_size)))
                {
                    menu_len = vts_data.vts_vob_sec_ - vts_data.vts_menu_vob_sec_;
                }
                else if (!title &&  (vts_data.vts_vob_sec_ + size_to_dvd_len(menu_size) +
                    size_to_dvd_len(info_size) - 1 < vts_data.last_vts_sec_))
                {
                    menu_len = vts_data.last_vts_sec_ - size_to_dvd_len(info_size) - vts_data.vts_menu_vob_sec_ + 1;
                }
                else
                {
                    menu_len = vts_data.vts_vob_sec_ - vts_data.vts_menu_vob_sec_;
                }

                if (menu_len > 0xffffffff)
                {
                    log_.print_line(ckT("error: menu VOB file larger than 4 million blocks."));
                    return false;
                }

                menu_node->data_pad_len_ = (ckcore::tuint32)menu_len - (ckcore::tuint32)size_to_dvd_len(menu_size);
            }

            // Find the actual size of VTS_XX_[1 to 9].VOB.
            ckcore::tuint64 title_len = 0;
            if (title)
            {
                title_len = vts_data.last_vts_sec_ + 1 - info_len -
                    menu_len - size_to_dvd_len(info_size);

                if (title_len > 0xffffffff)
                {
                    log_.print_line(ckT("error: title files larger than 4 million blocks."));
                    return false;
                }

                // We only pad the last title node (not sure if that is correct).
                FileTreeNode *pLastTitleNode = find_video_node(file_tree,FST_TITLE,counter);
                if (pLastTitleNode != NULL)
                    pLastTitleNode->data_pad_len_ = (ckcore::tuint32)title_len - (ckcore::tuint32)size_to_dvd_len(title_size);
            }

            // Find the actual size of VTS_XX_0.BUP.
            ckcore::tuint64 bkup_len;
            if (ts_sectors.size() > counter) {
                bkup_len = ts_sectors[counter] - ts_sectors[counter - 1] -
                    title_len - menu_len - info_len;
            }
            else
            {
                bkup_len = vts_data.last_vts_sec_ + 1 - title_len - menu_len - info_len;
            }

            if (bkup_len > 0xffffffff)
            {
                log_.print_line(ckT("error: BUP file larger than 4 million blocks."));
                return false;
            }

            FileTreeNode *bkup_node = find_video_node(file_tree,FST_BACKUP,counter);
            if (bkup_node != NULL)
                bkup_node->data_pad_len_ = (ckcore::tuint32)bkup_len - (ckcore::tuint32)size_to_dvd_len(info_size);

            // We're done.
            ifo_reader.close();

            // Increase the counter.
            counter++;
        }

        return true;
    }

    bool DvdVideo::print_file_padding(FileTree &file_tree)
    {
        log_.print_line(ckT("DvdVideo::print_file_padding"));

        FileTreeNode *vts_node = file_tree.get_node_from_path(ckT("/VIDEO_TS"));
        if (vts_node == NULL)
        {
            log_.print_line(ckT("error: unable to locate VIDEO_TS folder in file tree."));
            return false;
        }

        std::vector<FileTreeNode *>::const_iterator it_vidfile;
        for (it_vidfile = vts_node->children_.begin(); it_vidfile !=
            vts_node->children_.end(); it_vidfile++)
        {
            log_.print_line(ckT("  %s: pad %u sector(s)."),
                (*it_vidfile)->file_name_.c_str(),(*it_vidfile)->data_pad_len_);
        }

        return true;
    }

    bool DvdVideo::calc_file_padding(FileTree &file_tree)
    {
        // First locate VIDEO_TS.IFO.
        FileTreeNode *vts_node = file_tree.get_node_from_path(ckT("/VIDEO_TS/VIDEO_TS.IFO"));
        if (vts_node == NULL)
        {
            log_.print_line(ckT("error: unable to locate VIDEO_TS.IFO in file tree."));
            return false;
        }

        // Read and validate VIDEO_TS.INFO.
        IfoReader ifo_reader(vts_node->file_path_.c_str());
        if (!ifo_reader.open())
        {
            log_.print_line(ckT("error: unable to open and identify VIDEO_TS.IFO."));
            return false;
        }

        if (ifo_reader.get_type() != IfoReader::IT_VMG)
        {
            log_.print_line(ckT("error: VIDEO_TS.IFO is not of VMG format."));
            return false;
        }

        IfoVmgData vmg_data;
        if (!ifo_reader.read_vmg(vmg_data))
        {
            log_.print_line(ckT("error: unable to read VIDEO_TS.IFO VMG data."));
            return false;
        }

        // Make a vector of all title set vectors (instead of titles).
        std::vector<ckcore::tuint32> ts_sectors(vmg_data.titles_.size());
        std::vector<ckcore::tuint32>::const_iterator it_last =
            std::unique_copy(vmg_data.titles_.begin(),vmg_data.titles_.end(),ts_sectors.begin());
        ts_sectors.resize(it_last - ts_sectors.begin());

        // Sort the titles according to the start of the vectors.
        std::sort(ts_sectors.begin(),ts_sectors.end());

        if (!read_file_set_info_root(file_tree,vmg_data,ts_sectors))
        {
            log_.print_line(ckT("error: unable to obtain necessary information from VIDEO_TS.* files."));
            return false;
        }

        if (!read_file_set_info(file_tree,ts_sectors))
        {
            log_.print_line(ckT("error: unable to obtain necessary information from DVD-Video files."));
            return false;
        }

        return true;
    }
};
