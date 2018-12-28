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

#include "ckfilesystem/isowriter.hh"
#include "ckfilesystem/filesystemhelper.hh"

namespace ckfilesystem
{
    FileSystemHelper::FileSystemHelper(FileSystem &file_sys) :
        file_sys_(file_sys)
    {
    }

    FileSystemHelper::~FileSystemHelper()
    {
    }

    // Warning: This function duplicates some functionality in Udf.
    // file_name is assumed to be at least as long as req_file_name.
    void FileSystemHelper::calc_file_name(const ckcore::tchar *req_file_name,
                                          ckcore::tchar *file_name,bool is_dir)
    {
        bool is_iso = file_sys_.is_iso();
        bool is_udf = file_sys_.is_udf();
        bool is_joliet = file_sys_.is_joliet();

        if (is_udf)
        {
            size_t name_len = ckcore::string::astrlen(file_name);
            ckcore::string::astrncpy(file_name,req_file_name,name_len < (254 >> 1) ? name_len : (254 >> 1));        // One byte is reserved for compression descriptor.
        }
        else if (is_joliet)
        {
            unsigned char file_name_buf[ISOWRITER_FILENAME_BUFFER_SIZE + 1];
            unsigned char len = file_sys_.joliet_.write_file_name((unsigned char *)file_name_buf,req_file_name,is_dir);

#ifdef _UNICODE
            unsigned char file_name_pos = 0;
            for (unsigned char i = 0; i < len; i++)
            {
                file_name[i]  = file_name_buf[file_name_pos++] << 8;
                file_name[i] |= file_name_buf[file_name_pos++];
            }

            file_name[len] = '\0';
#else
            wchar_t utf_file_name[ISOWRITER_FILENAME_BUFFER_SIZE + 1];
            unsigned char file_name_pos = 0;
            for (unsigned char i = 0; i < len; i++)
            {
                utf_file_name[i]  = file_name_buf[file_name_pos++] << 8;
                utf_file_name[i] |= file_name_buf[file_name_pos++];
            }

            utf_file_name[len] = '\0';

            ckcore::string::utf16_to_ansi(utf_file_name,file_name,len);
#endif
        }
        else if (is_iso)
        {
#ifdef _UNICODE
            char ansi_file_name[ISOWRITER_FILENAME_BUFFER_SIZE + 1];
            unsigned char len = file_sys_.iso_.write_file_name((unsigned char *)ansi_file_name,req_file_name,is_dir);
            ansi_file_name[len] = '\0';

            ckcore::string::ansi_to_utf16(ansi_file_name,file_name,len + 1);
#else
            unsigned char len = file_sys_.iso_.write_file_name((unsigned char *)file_name,req_file_name,is_dir);
            file_name[len] = '\0';
#endif
        }
        else
        {
            ckcore::string::astrcpy(file_name,req_file_name);
        }
    }

    void FileSystemHelper::calc_file_path(const ckcore::tchar *req_file_path,
                                          ckcore::tstring &file_path)
    {
        size_t dir_path_len = ckcore::string::astrlen(req_file_path),prev_delim = 0,pos = 0;
        ckcore::tstring cur_dir_name;
        ckcore::tchar file_name_buf[ISOWRITER_FILENAME_BUFFER_SIZE + 1];

        // Locate the first delimiter (so we can safely skip any driveletter).
        for (size_t i = 0; i < dir_path_len; i++)
        {
            if (req_file_path[i] == '/' || req_file_path[i] == '\\')
            {
                pos = i;
                break;
            }
        }

        // Make sure that we don't delete any thing before the first delimiter that exists in file_path.
        if (pos > 0)
        {
            prev_delim = pos;
            
            if (file_path.length() > pos)
                file_path.erase(pos);
        }

        for (; pos < dir_path_len; pos++)
        {
            if (req_file_path[pos] == '/' || req_file_path[pos] == '\\')    // I don't like supporting two delimiters.
            {
                if (pos > (prev_delim + 1))
                {
                    // Obtain the name of the current directory.
                    cur_dir_name.erase();
                    for (size_t j = prev_delim + 1; j < pos; j++)
                        cur_dir_name.push_back(req_file_path[j]);

                    calc_file_name(cur_dir_name.c_str(),file_name_buf,true);
                    file_path.append(ckT("/"));
                    file_path.append(file_name_buf);
                }

                prev_delim = pos;
            }
        }

        calc_file_name(req_file_path + prev_delim + 1,file_name_buf,false);

        size_t file_name_buf_len = ckcore::string::astrlen(file_name_buf);
        if (file_name_buf[file_name_buf_len - 2] == ';')
            file_name_buf[file_name_buf_len - 2] = '\0';

        file_path.append(ckT("/"));
        file_path.append(file_name_buf);
    }
};
