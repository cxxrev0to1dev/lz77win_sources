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
#include <ckcore/log.hh>
#include "ckfilesystem/fileset.hh"
#include "ckfilesystem/iso.hh"
#include "ckfilesystem/joliet.hh"
#include "ckfilesystem/eltorito.hh"
#include "ckfilesystem/udf.hh"

namespace ckfilesystem
{
    class FileSystem
    {
    public:
        friend class IsoWriter;
        friend class UdfWriter;
        friend class FileSystemHelper;

    public:
        enum Type
        {
            TYPE_ISO,
            TYPE_ISO_JOLIET,
            TYPE_ISO_UDF,
            TYPE_ISO_UDF_JOLIET,
            TYPE_UDF,
            TYPE_DVDVIDEO
        };

    private:
        // What type of file system should be created.
        Type type_;

        // Different standard implementations.
        Iso iso_;
        Joliet joliet_;
        ElTorito eltorito_;
        Udf udf_;

        // File set.
        const FileSet &file_set_;

    public:
        FileSystem(Type type,const FileSet &file_set);
        ~FileSystem();

        // File system modifiers, mixed set for Joliet, UDF and ISO9660.
        void set_volume_label(const ckcore::tchar *label);
        void set_text_fields(const ckcore::tchar *sys_ident,
                             const ckcore::tchar *volset_ident,
                             const ckcore::tchar *publ_ident,
                             const ckcore::tchar *prep_ident);
        void set_file_fields(const ckcore::tchar *copy_file_ident,
                             const ckcore::tchar *abst_file_ident,
                             const ckcore::tchar *bibl_file_ident);
        void set_interchange_level(Iso::InterchangeLevel inter_level);
        void set_char_set(CharacterSet char_set);
        void set_include_file_ver_info(bool include);
        void set_part_access_type(Udf::PartAccessType access_type);
        void set_relax_max_dir_level(bool relax);
        void set_long_joliet_names(bool enable);

        bool add_boot_image_no_emu(const ckcore::tchar *full_path,bool bootable,
                                   ckcore::tuint16 load_segment,ckcore::tuint16 sec_count);
        bool add_boot_image_floppy(const ckcore::tchar *full_path,bool bootable);
        bool add_boot_image_hard_disk(const ckcore::tchar *full_path,bool bootable);

        // Information output routines.
        const FileSet &files();

        bool is_iso();
        bool is_joliet();
        bool is_udf();
        bool is_dvdvideo();

        bool allows_fragmentation();
        unsigned char get_max_dir_level();
    };
};
