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

#include <string.h>
#include <iostream>
#include <ckcore/string.hh>
#include "ckfilesystem/util.hh"
#include "ckfilesystem/joliet.hh"

namespace ckfilesystem
{
    using namespace util;

    Joliet::Joliet() : inc_file_ver_info_(true),max_name_len_(64)
    {
        init_vol_desc();
    }

    Joliet::~Joliet()
    {
    }

    /*
        Guaraties that the returned character is allowed by the Joliet file system.
    */
    wchar_t Joliet::make_char(wchar_t c)
    {
        if (c == '*' || c == '/' || c == ':' || c == ';' || c == '?' || c == '\\')
            return '_';

        return c;
    }

    /*
     * Find the last delimiter of the specified kind in the specified string.
     */
    int Joliet::last_delimiter_w(const wchar_t *str,wchar_t delim)
    {    
        int len = (int)wcslen(str);
        for (int i = len - 1; i >= 0; i--)
        {
            if (str[i] == delim)
                return i;
        }

        return -1;
    }

    /*
        Copies the source string to the target buffer assuring that all characters
        in the source string are allowed by the Joliet file system. iLen should
        be the length of the source string in wchar_t characters.
    */
    void Joliet::mem_str_cpy(unsigned char *target,const wchar_t *source,size_t len)
    {
        for (size_t i = 0,j = 0; j < len; j++)
        {
            wchar_t c = make_char(source[j]);
            target[i++] = c >> 8;
            target[i++] = c & 0xff;
        }
    }

    void Joliet::empty_str_buffer(unsigned char *buffer,size_t size)
    {
        for (size_t i = 0; i < size; i += 2)
        {
            buffer[0] = 0x00;
            buffer[1] = 0x20;
        }
    }

    void Joliet::init_vol_desc()
    {
        // Clear memory.
        memset(&voldesc_suppl_,0,sizeof(voldesc_suppl_));
        empty_str_buffer(voldesc_suppl_.sys_ident,sizeof(voldesc_suppl_.sys_ident));
        empty_str_buffer(voldesc_suppl_.vol_ident,sizeof(voldesc_suppl_.vol_ident));
        empty_str_buffer(voldesc_suppl_.volset_ident,sizeof(voldesc_suppl_.volset_ident));
        empty_str_buffer(voldesc_suppl_.publ_ident,sizeof(voldesc_suppl_.publ_ident));
        empty_str_buffer(voldesc_suppl_.prep_ident,sizeof(voldesc_suppl_.prep_ident));
        empty_str_buffer(voldesc_suppl_.app_ident,sizeof(voldesc_suppl_.app_ident));
        empty_str_buffer(voldesc_suppl_.copy_file_ident,sizeof(voldesc_suppl_.copy_file_ident));
        empty_str_buffer(voldesc_suppl_.abst_file_ident,sizeof(voldesc_suppl_.abst_file_ident));
        empty_str_buffer(voldesc_suppl_.bibl_file_ident,sizeof(voldesc_suppl_.bibl_file_ident));

        // Set primary volume descriptor header.
        voldesc_suppl_.type = VOLDESCTYPE_SUPPL_VOL_DESC;
        voldesc_suppl_.version = 1;
        voldesc_suppl_.file_struct_ver = 1;
        memcpy(voldesc_suppl_.ident,iso_ident_cd,sizeof(voldesc_suppl_.ident)); 

        // Always use Joliet level 3.
        voldesc_suppl_.esc_sec[0] = 0x25;
        voldesc_suppl_.esc_sec[1] = 0x2f;
        voldesc_suppl_.esc_sec[2] = 0x45;

        // Set the root directory record.
        voldesc_suppl_.root_dir_record.dir_record_len = 34;
        voldesc_suppl_.root_dir_record.file_flags = DIRRECORD_FILEFLAG_DIRECTORY;
        voldesc_suppl_.root_dir_record.file_ident_len = 1;  // One byte is always allocated in the tiso_dir_record structure.

        // Set application identifier.
        memset(voldesc_suppl_.app_data,0x20,sizeof(voldesc_suppl_.app_data));
        char app_ident[] = { 0x00,0x49,0x00,0x6e,0x00,0x66,0x00,0x72,0x00,0x61,
            0x00,0x52,0x00,0x65,0x00,0x63,0x00,0x6f,0x00,0x72,0x00,0x64,0x00,0x65,0x00,0x72,
            0x00,0x20,0x00,0x28,0x00,0x43,0x00,0x29,0x00,0x20,0x00,0x32,0x00,0x30,0x00,0x30,
            0x00,0x36,0x00,0x2d,0x00,0x32,0x00,0x30,0x00,0x30,0x00,0x38,0x00,0x20,0x00,0x43,
            0x00,0x68,0x00,0x72,0x00,0x69,0x00,0x73,0x00,0x74,0x00,0x69,0x00,0x61,0x00,0x6e,
            0x00,0x20,0x00,0x4b,0x00,0x69,0x00,0x6e,0x00,0x64,0x00,0x61,0x00,0x68,0x00,0x6c };
        memcpy(voldesc_suppl_.app_ident,app_ident,90);
    }

    void Joliet::write_vol_desc(ckcore::CanexOutStream &out_stream,struct tm &create_time,
                                ckcore::tuint32 vol_space_size,ckcore::tuint32 pathtable_size,
                                ckcore::tuint32 pos_pathtable_l,ckcore::tuint32 pos_pathtable_m,
                                ckcore::tuint32 root_extent_loc,ckcore::tuint32 ulDataLen)
    {
        // Initialize the supplementary volume descriptor.
        write733(voldesc_suppl_.vol_space_size,vol_space_size);     // Volume size in sectors.
        write723(voldesc_suppl_.volset_size,1);     // Only one disc in the volume set.
        write723(voldesc_suppl_.volseq_num,1);      // This is the first disc in the volume set.
        write723(voldesc_suppl_.logical_block_size,ISO_SECTOR_SIZE);
        write733(voldesc_suppl_.path_table_size,pathtable_size);    // Path table size in bytes.
        write731(voldesc_suppl_.path_table_type_l,pos_pathtable_l); // Start sector of LSBF path table.
        write732(voldesc_suppl_.path_table_type_m,pos_pathtable_m); // Start sector of MSBF path table.

        write733(voldesc_suppl_.root_dir_record.extent_loc,root_extent_loc);
        write733(voldesc_suppl_.root_dir_record.data_len,ulDataLen);
        write723(voldesc_suppl_.root_dir_record.volseq_num,1);  // The file extent is on the first volume set.

        // Time information.
        iso_make_datetime(create_time,voldesc_suppl_.root_dir_record.rec_timestamp);

        iso_make_datetime(create_time,voldesc_suppl_.create_time);
        memcpy(&voldesc_suppl_.modify_time,&voldesc_suppl_.create_time,sizeof(tiso_voldesc_datetime));

        memset(&voldesc_suppl_.expr_time,'0',sizeof(tiso_voldesc_datetime));
        voldesc_suppl_.expr_time.zone = 0x00;
        memset(&voldesc_suppl_.effect_time,'0',sizeof(tiso_voldesc_datetime));
        voldesc_suppl_.effect_time.zone = 0x00;

        // Write the supplementary volume descriptor.
        out_stream.write(&voldesc_suppl_,sizeof(voldesc_suppl_));
    }

    void Joliet::set_volume_label(const ckcore::tchar *label)
    {
        size_t label_len = ckcore::string::astrlen(label);
        size_t label_copy_len = label_len < 16 ? label_len : 16;

        empty_str_buffer(voldesc_suppl_.vol_ident,sizeof(voldesc_suppl_.vol_ident));

    #ifdef _UNICODE
        mem_str_cpy(voldesc_suppl_.vol_ident,label,label_copy_len);
    #else
        wchar_t utf_label[17];
        ckcore::string::ansi_to_utf16(label,utf_label,sizeof(utf_label) / sizeof(wchar_t));
        mem_str_cpy(voldesc_suppl_.vol_ident,utf_label,label_copy_len);
    #endif
    }

    void Joliet::set_text_fields(const ckcore::tchar *sys_ident,const ckcore::tchar *volset_ident,
                                 const ckcore::tchar *publ_ident,const ckcore::tchar *prep_ident)
    {
        size_t sys_ident_len = ckcore::string::astrlen(sys_ident);
        size_t volset_ident_len = ckcore::string::astrlen(volset_ident);
        size_t publ_ident_len = ckcore::string::astrlen(publ_ident);
        size_t prep_ident_len = ckcore::string::astrlen(prep_ident);

        size_t sys_ident_copy_len = sys_ident_len < 16 ? sys_ident_len : 16;
        size_t volset_ident_copy_len = volset_ident_len < 64 ? volset_ident_len : 64;
        size_t publ_ident_copy_len = publ_ident_len < 64 ? publ_ident_len : 64;
        size_t prep_ident_copy_len = prep_ident_len < 64 ? prep_ident_len : 64;

        empty_str_buffer(voldesc_suppl_.sys_ident,sizeof(voldesc_suppl_.sys_ident));
        empty_str_buffer(voldesc_suppl_.volset_ident,sizeof(voldesc_suppl_.volset_ident));
        empty_str_buffer(voldesc_suppl_.publ_ident,sizeof(voldesc_suppl_.publ_ident));
        empty_str_buffer(voldesc_suppl_.prep_ident,sizeof(voldesc_suppl_.prep_ident));

    #ifdef _UNICODE
        mem_str_cpy(voldesc_suppl_.sys_ident,sys_ident,sys_ident_copy_len);
        mem_str_cpy(voldesc_suppl_.volset_ident,volset_ident,volset_ident_copy_len);
        mem_str_cpy(voldesc_suppl_.publ_ident,publ_ident,publ_ident_copy_len);
        mem_str_cpy(voldesc_suppl_.prep_ident,prep_ident,prep_ident_copy_len);
    #else
        wchar_t utf_sys_ident[17];
        wchar_t utf_volset_ident[65];
        wchar_t utf_publ_ident[65];
        wchar_t utf_prep_ident[65];

        ckcore::string::ansi_to_utf16(sys_ident,utf_sys_ident,sizeof(utf_sys_ident) / sizeof(wchar_t));
        ckcore::string::ansi_to_utf16(volset_ident,utf_volset_ident,sizeof(utf_volset_ident) / sizeof(wchar_t));
        ckcore::string::ansi_to_utf16(publ_ident,utf_publ_ident,sizeof(utf_publ_ident) / sizeof(wchar_t));
        ckcore::string::ansi_to_utf16(prep_ident,utf_prep_ident,sizeof(utf_prep_ident) / sizeof(wchar_t));

        mem_str_cpy(voldesc_suppl_.sys_ident,utf_sys_ident,sys_ident_copy_len);
        mem_str_cpy(voldesc_suppl_.volset_ident,utf_volset_ident,volset_ident_copy_len);
        mem_str_cpy(voldesc_suppl_.publ_ident,utf_publ_ident,publ_ident_copy_len);
        mem_str_cpy(voldesc_suppl_.prep_ident,utf_prep_ident,prep_ident_copy_len);
    #endif
    }

    void Joliet::set_file_fields(const ckcore::tchar *copy_file_ident,
                                 const ckcore::tchar *abst_file_ident,
                                 const ckcore::tchar *bibl_file_ident)
    {
        size_t copy_file_ident_len = ckcore::string::astrlen(copy_file_ident);
        size_t abst_file_ident_len = ckcore::string::astrlen(abst_file_ident);
        size_t bibl_file_ident_len = ckcore::string::astrlen(bibl_file_ident);

        size_t copy_file_ident_copy_len = copy_file_ident_len < 18 ? copy_file_ident_len : 18;
        size_t abst_file_ident_copy_len = abst_file_ident_len < 18 ? abst_file_ident_len : 18;
        size_t bibl_file_ident_copy_len = bibl_file_ident_len < 18 ? bibl_file_ident_len : 18;

        empty_str_buffer(voldesc_suppl_.copy_file_ident,sizeof(voldesc_suppl_.copy_file_ident));
        empty_str_buffer(voldesc_suppl_.abst_file_ident,sizeof(voldesc_suppl_.abst_file_ident));
        empty_str_buffer(voldesc_suppl_.bibl_file_ident,sizeof(voldesc_suppl_.bibl_file_ident));

    #ifdef _UNICODE
        mem_str_cpy(voldesc_suppl_.copy_file_ident,copy_file_ident,copy_file_ident_copy_len);
        mem_str_cpy(voldesc_suppl_.abst_file_ident,abst_file_ident,abst_file_ident_copy_len);
        mem_str_cpy(voldesc_suppl_.bibl_file_ident,bibl_file_ident,bibl_file_ident_copy_len);
    #else
        wchar_t utf_copy_file_ident[19];
        wchar_t utf_abst_file_ident[19];
        wchar_t utf_bibl_file_ident[19];

        ckcore::string::ansi_to_utf16(copy_file_ident,utf_copy_file_ident,sizeof(utf_copy_file_ident) / sizeof(wchar_t));
        ckcore::string::ansi_to_utf16(abst_file_ident,utf_abst_file_ident,sizeof(utf_abst_file_ident) / sizeof(wchar_t));
        ckcore::string::ansi_to_utf16(bibl_file_ident,utf_bibl_file_ident,sizeof(utf_bibl_file_ident) / sizeof(wchar_t));

        mem_str_cpy(voldesc_suppl_.copy_file_ident,utf_copy_file_ident,copy_file_ident_copy_len);
        mem_str_cpy(voldesc_suppl_.abst_file_ident,utf_abst_file_ident,abst_file_ident_copy_len);
        mem_str_cpy(voldesc_suppl_.bibl_file_ident,utf_bibl_file_ident,bibl_file_ident_copy_len);
    #endif
    }

    void Joliet::set_include_file_ver_info(bool include)
    {
        inc_file_ver_info_ = include;
    }

    void Joliet::set_relax_max_name_len(bool relax)
    {
        if (relax)
            max_name_len_ = JOLIET_MAX_NAMELEN_RELAXED;
        else
            max_name_len_ = JOLIET_MAX_NAMELEN_NORMAL;
    }

    unsigned char Joliet::write_file_name(unsigned char *buffer,const ckcore::tchar *file_name,bool is_dir)
    {
#ifndef _UNICODE
        wchar_t utf_file_name[JOLIET_MAX_NAMELEN_RELAXED + 1];
        ckcore::string::ansi_to_utf16(file_name,utf_file_name,sizeof(utf_file_name) / sizeof(wchar_t));
#else
        const wchar_t *utf_file_name = file_name;
#endif

        int file_name_len = (int)wcslen(utf_file_name),max = 0;

        if (file_name_len > max_name_len_)
        {
            int ext_delim = last_delimiter_w(utf_file_name,'.');
            if (ext_delim != -1)
            {
                int ext_len = (int)file_name_len - ext_delim - 1;
                if (ext_len > max_name_len_ - 1)    // The file can at most contain an extension of length max_name_len_ - 1 characters.
                    ext_len = max_name_len_ - 1;

                // Copy the file name.
                max = ext_delim < (max_name_len_ - ext_len) ? ext_delim : (max_name_len_ - 1 - ext_len);
                mem_str_cpy(buffer,utf_file_name,max);

                int out_pos = max << 1;
                buffer[out_pos++] = 0x00;
                buffer[out_pos++] = '.';

                // Copy the extension.
                mem_str_cpy(buffer + out_pos,utf_file_name + ext_delim + 1,ext_len);

                max = max_name_len_;
            }
            else
            {
                max = max_name_len_;
                mem_str_cpy(buffer,utf_file_name,max);
            }
        }
        else
        {
            max = file_name_len;
            mem_str_cpy(buffer,utf_file_name,max);
        }

        if (!is_dir && inc_file_ver_info_)
        {
            int out_pos = max << 1;
            buffer[out_pos + 0] = 0x00;
            buffer[out_pos + 1] = ';';
            buffer[out_pos + 2] = 0x00;
            buffer[out_pos + 3] = '1';

            max += 2;
        }

        return max;
    }

    unsigned char Joliet::calc_file_name_len(const ckcore::tchar *file_name,bool is_dir)
    {
        size_t name_len = ckcore::string::astrlen(file_name);
        if (name_len >= (size_t)max_name_len_)
            name_len = max_name_len_;

        if (!is_dir && inc_file_ver_info_)
            name_len += 2;

        return (unsigned char)name_len;
    }

    /**
        Returns true if the file names includes the two character file version
        information (;1).
    */
    bool Joliet::includes_file_ver_info()
    {
        return inc_file_ver_info_;
    }
};
