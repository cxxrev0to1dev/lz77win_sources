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
#include <set>
#include <ckcore/types.hh>
#include <ckcore/string.hh>

namespace ckfilesystem
{
    /**
     * @brief Describes a file that should be included in the disc image.
     */
    class FileDescriptor
    {
    public:
        enum
        {
            FLAG_DIRECTORY = 0x01,
            FLAG_IMPORTED = 0x02
        };

        unsigned char flags_;
        ckcore::tstring internal_path_;     // Path in disc image.
        ckcore::tstring external_path_;     // Path on hard drive.

        void *data_ptr_;                    // Pointer to a user-defined structure, designed for IsoTreeNode.

        FileDescriptor(const ckcore::tchar *internal_path,const ckcore::tchar *external_path,
                       unsigned char flags = 0,void *data_ptr = NULL) :
            flags_(flags),
            internal_path_(internal_path),external_path_(external_path),
            data_ptr_(data_ptr)
        {
        }
    };

    /**
     * @brief Sorts the set of files according to the disc file system requirements.
     */
    class FileComparator
    {
    private:
        bool dvd_video_;

        /**
         * Returns a weight of the specified file name, a lighter file should
         * be placed heigher in the directory hierarchy.
         * @param [in] file_path File path to weight.
         * @return Weight of the file path.
         */
        ckcore::tuint32 entry_weight(const ckcore::tstring &file_path) const
        {
            ckcore::tuint32 weight = 0xffffffff;

            // Quick test for optimization.
            if (file_path.size() >= 9 && file_path[1] == 'V')
            {
                if (!ckcore::string::astrcmp(file_path.c_str(),ckT("/VIDEO_TS")))   // The VIDEO_TS folder should be first.
                    return 0;

                if (file_path.size() >= 10 &&
                    !ckcore::string::astrncmp(file_path.c_str(),ckT("/VIDEO_TS/"),10))
                {
                    const ckcore::tchar *file_name = file_path.c_str() + 10;

                    if (file_path.size() >= (10 + 9) &&
                        !ckcore::string::astrncmp(file_name,ckT("VIDEO_TS"),8))
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
                    else if (file_path.size() >= (10 + 4) &&
                        !ckcore::string::astrncmp(file_name,ckT("VTS_"),4))
                    {
                        weight -= 0x40000000;

                        // Just a safety measure.
                        if (file_path.size() < (10 + 64))
                        {
                            ckcore::tchar file_ext[64];
                            ckcore::tuint32 num = 0,sub_num = 0;

                            if (asscanf(file_name,ckT("VTS_%u_%u.%[^\0]"),&num,&sub_num,file_ext) == 3)
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
                }
            }

            return weight;
        }

    public:
        FileComparator(bool dvd_video) : dvd_video_(dvd_video)
        {
        }

        bool operator()(const FileDescriptor *item1,const FileDescriptor *item2) const
        {
            if (dvd_video_)
            {
                ckcore::tuint32 weight1 = entry_weight(item1->internal_path_);
                ckcore::tuint32 weight2 = entry_weight(item2->internal_path_);

                if (weight1 != weight2)
                    return weight1 < weight2;
            }

            return item1->internal_path_.compare(item2->internal_path_) < 0;
        }
    };

    typedef std::set<ckfilesystem::FileDescriptor *,ckfilesystem::FileComparator> FileSet;

    inline void destroy_file_set(FileSet &fileset)
    {
        for (FileSet::const_iterator it = fileset.begin(); it != fileset.end(); it++)
            delete *it;

        fileset.clear();
    }
};
