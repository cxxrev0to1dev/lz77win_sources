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

#include "ckfilesystem/filesystem.hh"

namespace ckfilesystem
{
    FileSystem::FileSystem(Type type,const FileSet &file_set) :
        type_(type),udf_(type == TYPE_DVDVIDEO),file_set_(file_set)
    {
    }

    FileSystem::~FileSystem()
    {
    }

    void FileSystem::set_volume_label(const ckcore::tchar *label)
    {
        iso_.set_volume_label(label);
        joliet_.set_volume_label(label);
        udf_.set_volume_label(label);
    }

    void FileSystem::set_text_fields(const ckcore::tchar *sys_ident,
                                          const ckcore::tchar *volset_ident,
                                          const ckcore::tchar *publ_ident,
                                          const ckcore::tchar *prep_ident)
    {
        iso_.set_text_fields(sys_ident,volset_ident,publ_ident,prep_ident);
        joliet_.set_text_fields(sys_ident,volset_ident,publ_ident,prep_ident);
    }

    void FileSystem::set_file_fields(const ckcore::tchar *copy_file_ident,
                                          const ckcore::tchar *abst_file_ident,
                                          const ckcore::tchar *bibl_file_ident)
    {
        iso_.set_file_fields(copy_file_ident,abst_file_ident,bibl_file_ident);
        joliet_.set_file_fields(copy_file_ident,abst_file_ident,bibl_file_ident);
    }

    void FileSystem::set_interchange_level(Iso::InterchangeLevel inter_level)
    {
        iso_.set_interchange_level(inter_level);
    }

    void FileSystem::set_char_set(CharacterSet char_set)
    {
        iso_.set_char_set(char_set);
    }

    void FileSystem::set_include_file_ver_info(bool include)
    {
        iso_.set_include_file_ver_info(include);
        joliet_.set_include_file_ver_info(include);
    }

    void FileSystem::set_part_access_type(Udf::PartAccessType access_type)
    {
        udf_.set_part_access_type(access_type);
    }

    void FileSystem::set_relax_max_dir_level(bool relax)
    {
        iso_.set_relax_max_dir_level(relax);
    }

    void FileSystem::set_long_joliet_names(bool enable)
    {
        joliet_.set_relax_max_name_len(enable);
    }

    bool FileSystem::add_boot_image_no_emu(const ckcore::tchar *full_path,bool bootable,
                                           ckcore::tuint16 load_segment,ckcore::tuint16 sec_count)
    {
        return eltorito_.add_boot_image_no_emu(full_path,bootable,load_segment,sec_count);
    }

    bool FileSystem::add_boot_image_floppy(const ckcore::tchar *full_path,bool bootable)
    {
        return eltorito_.add_boot_image_floppy(full_path,bootable);
    }

    bool FileSystem::add_boot_image_hard_disk(const ckcore::tchar *full_path,bool bootable)
    {
        return eltorito_.add_boot_image_hard_disk(full_path,bootable);
    }

    const FileSet &FileSystem::files()
    {
        return file_set_;
    }

    bool FileSystem::is_iso()
    {
        return type_ != TYPE_UDF;
    }

    bool FileSystem::is_joliet()
    {
        return type_ == TYPE_ISO_JOLIET || type_ == TYPE_ISO_UDF_JOLIET;
    }

    bool FileSystem::is_udf()
    {
        return type_ == TYPE_ISO_UDF || type_ == TYPE_ISO_UDF_JOLIET ||
               type_ == TYPE_UDF || type_ == TYPE_DVDVIDEO;
    }

    bool FileSystem::is_dvdvideo()
    {
        return type_ == TYPE_DVDVIDEO;
    }

    bool FileSystem::allows_fragmentation()
    {
        return iso_.allows_fragmentation();
    }

    unsigned char FileSystem::get_max_dir_level()
    {
        return iso_.get_max_dir_level();
    }
};
