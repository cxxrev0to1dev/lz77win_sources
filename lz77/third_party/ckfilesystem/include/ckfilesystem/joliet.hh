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
#include <ckcore/types.hh>
#include <ckcore/stream.hh>
#include <ckcore/canexstream.hh>
#include "ckfilesystem/iso.hh"

#define JOLIET_MAX_NAMELEN_NORMAL            64     // According to Joliet specification.
#define JOLIET_MAX_NAMELEN_RELAXED          101     // 207 bytes = 101 wide characters + 4 wide characters for file version.

namespace ckfilesystem
{
    class Joliet
    {
    private:
        bool inc_file_ver_info_;
        int max_name_len_;

        tiso_voldesc_suppl voldesc_suppl_;

        wchar_t make_char(wchar_t c);
        int last_delimiter_w(const wchar_t *str,wchar_t delim);
        void mem_str_cpy(unsigned char *target,const wchar_t *source,size_t len);
        void empty_str_buffer(unsigned char *buffer,size_t size);

        void init_vol_desc();

    public:
        Joliet();
        ~Joliet();

        // Change of internal state functions.
        void set_volume_label(const ckcore::tchar *label);
        void set_text_fields(const ckcore::tchar *sys_ident,
                             const ckcore::tchar *volset_ident,
                             const ckcore::tchar *publ_ident,
                             const ckcore::tchar *prep_ident);
        void set_file_fields(const ckcore::tchar *copy_file_ident,
                             const ckcore::tchar *abst_file_ident,
                             const ckcore::tchar *bibl_file_ident);
        void set_include_file_ver_info(bool include);
        void set_relax_max_name_len(bool relax);

        // Write functions.
        void write_vol_desc(ckcore::CanexOutStream &out_stream,struct tm &create_time,
                            ckcore::tuint32 vol_space_size,ckcore::tuint32 pathtable_size,
                            ckcore::tuint32 pos_pathtable_l,ckcore::tuint32 pos_pathtable_m,
                            ckcore::tuint32 root_extent_loc,ckcore::tuint32 data_len);

        // Helper functions.
        unsigned char write_file_name(unsigned char *buffer,const ckcore::tchar *file_name,bool is_dir);
        unsigned char calc_file_name_len(const ckcore::tchar *file_name,bool is_dir);
        bool includes_file_ver_info();
    };
};
