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

#ifdef _WINDOWS
#include <windows.h>
#endif
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <ckcore/string.hh>
#include <ckcore/memory.hh>
#include <ckcore/exception.hh>
#include "ckfilesystem/iso.hh"
#include "ckfilesystem/util.hh"
#include "ckfilesystem/udf.hh"

namespace ckfilesystem
{
    using namespace util;

    /*
        Identifiers.
    */
    const unsigned char ident_udf_charset[] = {
        0x4f,0x53,0x54,0x41,0x20,0x43,0x6f,0x6d,0x70,0x72,0x65,0x73,
        0x73,0x65,0x64,0x20,0x55,0x6e,0x69,0x63,0x6f,0x64,0x65 };
    const unsigned char ident_udf_entity_compliant[] = {
        0x2a,0x4f,0x53,0x54,0x41,0x20,0x55,0x44,0x46,0x20,0x43,0x6f,
        0x6d,0x70,0x6c,0x69,0x61,0x6e,0x74 };

    // Used.
    const unsigned char ident_udf_entity_lv_info[] = {
        0x2a,0x55,0x44,0x46,0x20,0x4c,0x56,0x20,0x49,0x6e,0x66,0x6f };
    const unsigned char ident_udf_entity_domain[] = {
        0x2a,0x4f,0x53,0x54,0x41,0x20,0x55,0x44,0x46,0x20,0x43,0x6f,
        0x6d,0x70,0x6c,0x69,0x61,0x6e,0x74 };
    const unsigned char ident_udf_free_ea_space[] = {
        0x2a,0x55,0x44,0x46,0x20,0x46,0x72,0x65,0x65,0x45,0x41,0x53,
        0x70,0x61,0x63,0x65 };
    const unsigned char ident_udf_cgms[] = {
        0x2a,0x55,0x44,0x46,0x20,0x44,0x56,0x44,0x20,0x43,0x47,0x4d,
        0x53,0x20,0x49,0x6e,0x66,0x6f };

    const unsigned char ident_udf_entity_free_ea_space[] = {
        0x2a,0x55,0x44,0x46,0x20,0x46,0x72,0x65,0x65,0x45,0x41,0x53,
        0x70,0x61,0x63,0x65 };
    const unsigned char ident_udf_entity_free_app_ea_space[] = {
        0x2a,0x55,0x44,0x46,0x20,0x46,0x72,0x65,0x65,0x41,0x70,0x70,
        0x45,0x41,0x53,0x70,0x61,0x63,0x65 };
    const unsigned char ident_udf_entity_dvd_cgms_info[] = {
        0x2a,0x55,0x44,0x46,0x20,0x44,0x56,0x44,0x20,0x43,0x47,0x4d,
        0x53,0x20,0x49,0x6e,0x66,0x6f };
    const unsigned char ident_udf_entity_os2ea[] = {
        0x2a,0x55,0x44,0x46,0x41,0x20,0x45,0x41};
    const unsigned char ident_udf_entity_os2ea_len[] = {
        0x2a,0x55,0x44,0x46,0x20,0x45,0x41,0x4c,0x65,0x6e,0x67,0x74,
        0x68 };
    const unsigned char ident_udf_entity_mac_vol_info[] = {
        0x2a,0x55,0x44,0x46,0x20,0x4d,0x61,0x63,0x20,0x56,0x6f,0x6c,
        0x75,0x6d,0x65,0x49,0x6e,0x66,0x6f };
    const unsigned char ident_udf_entity_mac_finder_info[] = {
        0x2a,0x55,0x44,0x46,0x20,0x4d,0x61,0x63,0x20,0x49,0x69,0x6e,
        0x64,0x65,0x72,0x49,0x6e,0x66,0x6f };
    const unsigned char ident_udf_entity_mac_unique_table[] = {
        0x2a,0x55,0x44,0x46,0x20,0x4d,0x61,0x63,0x20,0x55,0x6e,0x69,
        0x71,0x75,0x65,0x49,0x44,0x54,0x61,0x62,0x6c,0x65 };
    const unsigned char ident_udf_entity_mac_res_fork[] = {
        0x2a,0x55,0x44,0x46,0x20,0x4d,0x61,0x63,0x20,0x52,0x65,0x73,
        0x6f,0x75,0x72,0x63,0x65,0x46,0x6f,0x72,0x6b };

    // Initial volume descriptor identifiers.
    const char *ident_bea = "BEA01";
    const char *ident_nsr = "NSR02";
    const char *ident_tea = "TEA01";

    const char *ident_part_content_fdc = "+FDC01";  // As if it were a volume recorded according to ECMA-107.
    const char *ident_part_content_cd  = "+CD001";  // As if it were a volume recorded according to ECMA-119.
    const char *ident_part_content_cdw = "+CDW02";  // As if it were a volume recorded according to ECMA-168.
    const char *ident_part_content_nsr = "+NSR02";  // According to Part 4 of this ECMA Standard.

    Udf::Udf(bool dvd_video) : crc_stream_(ckcore::CrcStream::ckCRC_CCITT),dvd_video_(dvd_video)
    {
        // Intialize the byte buffer.
        byte_buffer_ = NULL;
        byte_buffer_size_ = 0;

        // Default parition type is read only.
        part_access_type_ = AT_READONLY;

        init_vol_desc_primary();
        init_vol_desc_partition();
        init_vol_desc_logical();
    }

    Udf::~Udf()
    {
        // Free the byte buffer.
        if (byte_buffer_ != NULL)
        {
            delete [] byte_buffer_;
            byte_buffer_ = NULL;

            byte_buffer_size_ = 0;
        }
    }

    void Udf::alloc_byte_buffer(ckcore::tuint32 min_size)
    {
        if (byte_buffer_size_ < min_size)
        {
            if (byte_buffer_ != NULL)
                delete [] byte_buffer_;

            byte_buffer_size_ = min_size;
            byte_buffer_ = new unsigned char[byte_buffer_size_];
        }
    }

    /*
        Takes a string of unicode wide characters and returns an OSTA CS0
        compressed unicode string. The unicode MUST be in the byte order of
        the compiler in order to obtain correct results. Returns an error
        if the compression ID is invalid.

        Note: This routine assumes the implementation already knows, by
        the local environment, how many bits are appropriate and therefore does
        no checking to test if the input characters fit into that number of
        bits or not.

        The function returns the total number of bytes in the compressed OSTA
        CS0 string, including the compression ID. -1 is returned if the
        compression ID is invalid.
    */
    size_t Udf::compress_unicode_str(size_t num_chars,unsigned char comp_id,
                                     const wchar_t *in_str,unsigned char *out_str)
    {
        if (comp_id != 8 && comp_id != 16)
            return -1;

        // Place compression code in first byte.
        out_str[0] = comp_id;

        size_t byte_index = 1,iUnicodeIndex = 0;
        while (iUnicodeIndex < num_chars)
        {
            if (comp_id == 16)
            {
                // First, place the high bits of the char into the byte stream.
                out_str[byte_index++] = (in_str[iUnicodeIndex] & 0xff00) >> 8;
            }
            // Then place the low bits into the stream.
            out_str[byte_index++] = in_str[iUnicodeIndex] & 0x00ff;
            iUnicodeIndex++;
        }

        return byte_index;
    }

    /*
        Helper function for filling a tudf_charspec structure.
    */
    void Udf::make_char_spec(tudf_charspec &char_spec)
    {
        memset(&char_spec,0,sizeof(tudf_charspec));
        char_spec.charset_type = 0;
        memcpy(char_spec.charset_info,ident_udf_charset,sizeof(ident_udf_charset));
    }

    /*
        Helper function for filling a tudf_intity_ident structure.
    */
    void Udf::make_ident(tudf_intity_ident &impl_ident,IdentType ident_type)
    {
        impl_ident.flags = 0;
        memset(impl_ident.ident,0,sizeof(impl_ident.ident));
        memset(impl_ident.ident_suffix,0,sizeof(impl_ident.ident_suffix));

        unsigned char os_class,os_ident;
        make_os_identifiers(os_class,os_ident);

        switch (ident_type)
        {
            case IT_DEVELOPER:
                {
                    char app_ident[] = { 0x2a,0x49,0x6e,0x66,0x72,0x61,0x52,0x65,0x63,0x6f,0x72,0x64,0x65,0x72 };
                    memcpy(impl_ident.ident,app_ident,sizeof(app_ident));

                    impl_ident.ident_suffix[0] = os_class;
                    impl_ident.ident_suffix[1] = os_ident;
                }
                break;

            case IT_LVINFO:
                memcpy(impl_ident.ident,ident_udf_entity_lv_info,sizeof(ident_udf_entity_lv_info));

                write721(impl_ident.ident_suffix,0x0102);   // Currently only UDF 1.02 is supported.

                impl_ident.ident_suffix[2] = os_class;
                impl_ident.ident_suffix[3] = os_ident;
                break;

            case IT_DOMAIN:
                memcpy(impl_ident.ident,ident_udf_entity_domain,sizeof(ident_udf_entity_domain));

                write721(impl_ident.ident_suffix,0x0102);   // Currently only UDF 1.02 is supported.

                impl_ident.ident_suffix[2] = UDF_DOMAIN_FLAG_HARD_WRITEPROTECT | UDF_DOMAIN_FLAG_SOFT_WRITEPROTECT;
                impl_ident.ident_suffix[3] = os_ident;
                break;

            case IT_FREEEASPACE:
                memcpy(impl_ident.ident,ident_udf_free_ea_space,sizeof(ident_udf_free_ea_space));

                write721(impl_ident.ident_suffix,0x0102);   // Currently only UDF 1.02 is supported.
                break;

            case IT_CGMS:
                memcpy(impl_ident.ident,ident_udf_cgms,sizeof(ident_udf_cgms));

                write721(impl_ident.ident_suffix,0x0102);   // Currently only UDF 1.02 is supported.
                break;
        }
    }

    /*
        Helper function for creating a tag.
    */
    void Udf::make_tag(tudf_tag &tag,ckcore::tuint16 ident)
    {
        memset(&tag,0,sizeof(tudf_tag));
        tag.tag_ident = ident;
        tag.desc_ver = UDF_TAG_DESCRIPTOR_VERSION;
        tag.tag_serial_num = 1;
        tag.desc_crc = 0;
        tag.desc_crc_len = 0;
    }

    /*
        Helper function for calculating descriptor CRC and tag checksum.
    */
    void Udf::make_tag_checksums(tudf_tag &tag,unsigned char *buffer)
    {
        crc_stream_.reset();
        crc_stream_.write(buffer,tag.desc_crc_len);
        tag.desc_crc = (ckcore::tuint16)crc_stream_.checksum();

        // Sum of bytes 0-3 and 5-15 modulo 256.
        unsigned char checksum = 0;
        for (unsigned i = 0; i < sizeof(tudf_tag); i++)
            checksum += ((unsigned char *)&tag)[i];

        tag.tag_chksum = checksum;
    }

    ckcore::tuint16 Udf::make_ext_addr_checksum(unsigned char *buffer)
    {
        ckcore::tuint16 checksum = 0;
        for (unsigned int i = 0; i < 48; i++)
            checksum += *buffer++;

        return checksum;
    }

    /*
        Generates a unique volume set indentifer to identify a particular
        volume. pVolSysIdent is assumed to hold 128 bytes.
    */
    void Udf::make_vol_set_ident(unsigned char *volset_ident,size_t volset_ident_size)
    {
        if (volset_ident_size < 18)
            return;

        ckcore::tchar charset[] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };
        wchar_t gen_ident[16];
        for (unsigned int i = 0; i < sizeof(gen_ident)/sizeof(wchar_t); i++)
            gen_ident[i] = charset[rand() % 16];

        // Make a compatible D-string.
        memset(volset_ident,0,volset_ident_size);

        unsigned char byte_len = (unsigned char)compress_unicode_str(16,
            UDF_COMPRESSION_BYTE,gen_ident,volset_ident);

        volset_ident[volset_ident_size - 1] = byte_len;
    }

    void Udf::make_date_time(struct tm &time,tudf_timestamp &udf_time)
    {
        udf_time.type_tz = 1 << 12;     // Type: local time.

#ifdef _WINDOWS
        TIME_ZONE_INFORMATION tzi;
        GetTimeZoneInformation(&tzi);
        udf_time.type_tz |= -tzi.Bias;
#else
        // FIXME: Add support for Unix time zones.
#endif

        udf_time.year = time.tm_year + 1900;
        udf_time.mon = time.tm_mon + 1;
        udf_time.day = time.tm_mday;
        udf_time.hour = time.tm_hour;
        udf_time.min = time.tm_min;
        udf_time.sec = time.tm_sec;
        udf_time.centisec = 0;
        udf_time.hundreds_of_microsec = 0;
        udf_time.microsec = 0;
    }

    void Udf::make_os_identifiers(unsigned char &os_class,unsigned char &os_ident)
    {
        // UDF 1.02 does not support any Windows identification.
        os_class = UDF_OSCLASS_UNDEFINED;
        os_ident = UDF_OSIDENT_UNDEFINED;
    }

    unsigned char Udf::make_file_ident(unsigned char *out_buffer,const ckcore::tchar *file_name)
    {
        size_t name_len = ckcore::string::astrlen(file_name);
        size_t copy_len = name_len < (254 >> 1) ? name_len : (254 >> 1);        // One byte is reserved for compression descriptor.

        // DVD-Video should use 8 bits to represent one character.
        unsigned char str_comp = dvd_video_ ? UDF_COMPRESSION_BYTE : UDF_COMPRESSION_UNICODE;

    #ifdef _UNICODE
        unsigned char byte_len = (unsigned char)compress_unicode_str(copy_len,str_comp,
            file_name,out_buffer);
    #else
        wchar_t szWideFileName[125];
        ckcore::string::ansi_to_utf16(file_name,szWideFileName,sizeof(szWideFileName) / sizeof(wchar_t));

        unsigned char byte_len = (unsigned char)compress_unicode_str(copy_len,str_comp,
            szWideFileName,out_buffer);
    #endif
        return byte_len;
    }

    void Udf::init_vol_desc_primary()
    {
        memset(&voldesc_primary_,0,sizeof(tudf_voldesc_prim));

        // Other members.
        make_ident(voldesc_primary_.impl_ident,IT_DEVELOPER);
        make_ident(voldesc_primary_.app_ident,IT_DEVELOPER);

        make_char_spec(voldesc_primary_.desc_charset);
        make_char_spec(voldesc_primary_.explanatory_charset);

        make_vol_set_ident(voldesc_primary_.volset_ident,sizeof(voldesc_primary_.volset_ident));
    }

    void Udf::init_vol_desc_partition()
    {
        memset(&voldesc_partition_,0,sizeof(tudf_voldesc_part));

        // Other members.
        voldesc_partition_.part_flags = UDF_PARTITION_FLAG_ALLOCATED;
        voldesc_partition_.part_num = 0;        // We always create the first parition using these routines.

        voldesc_partition_.part_content_ident.flags = dvd_video_ ? UDF_ENTITYFLAG_DVDVIDEO : 0;
        memcpy(voldesc_partition_.part_content_ident.ident,ident_part_content_nsr,strlen(ident_part_content_nsr));

        switch (part_access_type_)
        {
            case AT_UNKNOWN:
                voldesc_partition_.access_type = UDF_PARTITION_ACCESS_UNKNOWN;
                break;
            case AT_WRITEONCE:
                voldesc_partition_.access_type = UDF_PARTITION_ACCESS_WRITEONCE;
                break;
            case AT_REWRITABLE:
                voldesc_partition_.access_type = UDF_PARTITION_ACCESS_REWRITABLE;
                break;
            case AT_OVERWRITABLE:
                voldesc_partition_.access_type = UDF_PARTITION_ACCESS_OVERWRITABLE;
                break;
            //case AT_READONLY:
            default:
                voldesc_partition_.access_type = UDF_PARTITION_ACCESS_READONLY;
                break;
        }

        make_ident(voldesc_partition_.impl_ident,IT_DEVELOPER);
    }

    void Udf::init_vol_desc_logical()
    {
        memset(&voldesc_logical_,0,sizeof(tudf_voldesc_logical));

        // Other members.
        make_char_spec(voldesc_logical_.desc_charset);

        voldesc_logical_.logical_block_size = UDF_SECTOR_SIZE;

        make_ident(voldesc_logical_.domain_ident,IT_DOMAIN);
        make_ident(voldesc_logical_.impl_ident,IT_DEVELOPER);
    
        write731(voldesc_logical_.logocal_vol_contents_use,UDF_SECTOR_SIZE);    // ?
    }

    void Udf::set_volume_label(const ckcore::tchar *label)
    {
        size_t label_len = ckcore::string::astrlen(label);
        size_t primary_copy_len = label_len < 15 ? label_len : 15;  // Two bytes are reserved for string format.
        size_t logical_copy_len = label_len < 63 ? label_len : 63;  // Two bytes are reserved for string format.

        // We need to update both the logical and primary descriptor identifiers.
        memset(voldesc_primary_.vol_ident,0,sizeof(voldesc_primary_.vol_ident));
        memset(voldesc_logical_.logical_vol_ident,0,sizeof(voldesc_logical_.logical_vol_ident));

        // DVD-Video should use 8 bits to represent one character.
        unsigned char str_comp = dvd_video_ ? UDF_COMPRESSION_BYTE : UDF_COMPRESSION_UNICODE;

    #ifdef _UNICODE
        unsigned char byte_len = (unsigned char)compress_unicode_str(primary_copy_len,str_comp,
            label,voldesc_primary_.vol_ident);
        voldesc_primary_.vol_ident[31] = byte_len;

        byte_len = (unsigned char)compress_unicode_str(logical_copy_len,str_comp,
            label,voldesc_logical_.logical_vol_ident);
        voldesc_logical_.logical_vol_ident[127] = byte_len;
    #else
        wchar_t szWidePrimaryLabel[17];
        ckcore::string::ansi_to_utf16(label,szWidePrimaryLabel,sizeof(szWidePrimaryLabel) / sizeof(wchar_t));

        unsigned char byte_len = (unsigned char)compress_unicode_str(primary_copy_len,str_comp,
            szWidePrimaryLabel,voldesc_primary_.vol_ident);
        voldesc_primary_.vol_ident[31] = byte_len;

        wchar_t szWideLogicalLabel[17];
        ckcore::string::ansi_to_utf16(label,szWideLogicalLabel,sizeof(szWideLogicalLabel) / sizeof(wchar_t));
        byte_len = (unsigned char)compress_unicode_str(logical_copy_len,str_comp,
            szWideLogicalLabel,voldesc_logical_.logical_vol_ident);
        voldesc_logical_.logical_vol_ident[127] = byte_len;
    #endif
    }

    void Udf::set_part_access_type(PartAccessType access_type)
    {
        part_access_type_ = access_type;
    }

    /*
        Write the initial volume descriptors. They're of the same format as the
        ISO9660 volume descriptors.
    */
    void Udf::write_vol_desc_initial(ckcore::CanexOutStream &out_stream)
    {
        tudf_volstruct_desc vol_struct_desc;
        memset(&vol_struct_desc,0,sizeof(tudf_volstruct_desc));
        vol_struct_desc.type = 0;
        vol_struct_desc.struct_ver = 1;

        memcpy(vol_struct_desc.ident,ident_bea,sizeof(vol_struct_desc.ident));
        out_stream.write(&vol_struct_desc,sizeof(tudf_volstruct_desc));

        memcpy(vol_struct_desc.ident,ident_nsr,sizeof(vol_struct_desc.ident));
        out_stream.write(&vol_struct_desc,sizeof(tudf_volstruct_desc));

        memcpy(vol_struct_desc.ident,ident_tea,sizeof(vol_struct_desc.ident));
        out_stream.write(&vol_struct_desc,sizeof(tudf_volstruct_desc));
    }

    void Udf::write_vol_desc_primary(ckcore::CanexOutStream &out_stream,ckcore::tuint32 voldesc_seqnum,
                                     ckcore::tuint32 sec_location,struct tm &create_time)
    {
        // Make the tag.
        make_tag(voldesc_primary_.desc_tag,UDF_TAGIDENT_PRIMVOLDESC);
        voldesc_primary_.desc_tag.tag_loc = sec_location;
        voldesc_primary_.desc_tag.desc_crc_len = sizeof(tudf_voldesc_prim) - sizeof(tudf_tag);

        // Update the primary volume descriptor data.
        voldesc_primary_.voldesc_seqnum = voldesc_seqnum;
        voldesc_primary_.voldesc_primnum = voldesc_seqnum;

        voldesc_primary_.volseq_num = 1;        // This is the first disc in the volume set.
        voldesc_primary_.max_volseq_num = 1;

        voldesc_primary_.interchange_level = UDF_INTERCHANGE_LEVEL_SINGLESET;
        voldesc_primary_.max_interchange_level = UDF_INTERCHANGE_LEVEL_SINGLESET;
        voldesc_primary_.charset_list = 1;
        voldesc_primary_.max_charset_list = 1;

        make_date_time(create_time,voldesc_primary_.rec_timestamp);

        // Calculate checksums.
        make_tag_checksums(voldesc_primary_.desc_tag,(unsigned char *)(&voldesc_primary_) + sizeof(tudf_tag));

        out_stream.write(&voldesc_primary_,sizeof(tudf_voldesc_prim));

        // Pad the sector from 512 to 2048 bytes.
        char tmp[1] = { 0 };
        for (ckcore::tuint32 i = 0; i < (UDF_SECTOR_SIZE - sizeof(tudf_voldesc_prim)); i++)
            out_stream.write(tmp,1);
    }

    void Udf::write_vol_desc_impl_use(ckcore::CanexOutStream &out_stream,
                                      ckcore::tuint32 voldesc_seqnum,
                                      ckcore::tuint32 sec_location)
    {   
        tudf_voldesc_impl_use impl_use_voldesc;
        memset(&impl_use_voldesc,0,sizeof(tudf_voldesc_impl_use));

        // Create tag.
        make_tag(impl_use_voldesc.desc_tag,UDF_TAGIDENT_IMPLUSEVOLDESC);
        impl_use_voldesc.desc_tag.tag_loc = sec_location;
        impl_use_voldesc.desc_tag.desc_crc_len = sizeof(tudf_voldesc_impl_use) - sizeof(tudf_tag);

        make_ident(impl_use_voldesc.impl_ident,IT_LVINFO);

        impl_use_voldesc.voldesc_seqnum = voldesc_seqnum;

        make_char_spec(impl_use_voldesc.lv_info.lv_info_charset);
        memcpy(impl_use_voldesc.lv_info.lv_ident,voldesc_logical_.logical_vol_ident,
            sizeof(impl_use_voldesc.lv_info.lv_ident));     // Steal the value from the logical descriptor.
        make_ident(impl_use_voldesc.lv_info.impl_ident,IT_DEVELOPER);

        // Calculate tag checksums.
        make_tag_checksums(impl_use_voldesc.desc_tag,(unsigned char *)(&impl_use_voldesc) + sizeof(tudf_tag));

        out_stream.write(&impl_use_voldesc,sizeof(tudf_voldesc_impl_use));

        // Pad the sector from 512 to 2048 bytes.
        char tmp[1] = { 0 };
        for (ckcore::tuint32 i = 0; i < (UDF_SECTOR_SIZE - sizeof(tudf_voldesc_impl_use)); i++)
            out_stream.write(tmp,1);
    }

    /**
        @param part_len is the partition size in sectors.
    */
    void Udf::write_vol_desc_partition(ckcore::CanexOutStream &out_stream,ckcore::tuint32 voldesc_seqnum,
                                       ckcore::tuint32 sec_location,ckcore::tuint32 part_start_loc,
                                       ckcore::tuint32 part_len)
    {
        // Make the tag.
        make_tag(voldesc_partition_.desc_tag,UDF_TAGIDENT_PARTDESC);
        voldesc_partition_.desc_tag.tag_loc = sec_location;
        voldesc_partition_.desc_tag.desc_crc_len = sizeof(tudf_voldesc_part) - sizeof(tudf_tag);

        voldesc_partition_.voldesc_seqnum = voldesc_seqnum;
        voldesc_partition_.part_start_loc = part_start_loc;
        voldesc_partition_.part_len = part_len;

        // Calculate tag checksums.
        make_tag_checksums(voldesc_partition_.desc_tag,(unsigned char *)(&voldesc_partition_) + sizeof(tudf_tag));

        out_stream.write(&voldesc_partition_,sizeof(tudf_voldesc_part));

        // Pad the sector from 512 to 2048 bytes.
        char tmp[1] = { 0 };
        for (ckcore::tuint32 i = 0; i < (UDF_SECTOR_SIZE - sizeof(tudf_voldesc_part)); i++)
            out_stream.write(tmp,1);
    }

    void Udf::write_vol_desc_logical(ckcore::CanexOutStream &out_stream,ckcore::tuint32 voldesc_seqnum,
                                     ckcore::tuint32 sec_location,tudf_extent_ad &integrity_seq_extent)
    {
        // Make the tag.
        make_tag(voldesc_logical_.desc_tag,UDF_TAGIDENT_LOGICALVOLDESC);
        voldesc_logical_.desc_tag.tag_loc = sec_location;
        voldesc_logical_.desc_tag.desc_crc_len = sizeof(tudf_voldesc_logical) +
            sizeof(tudf_logical_partmap_type1) - sizeof(tudf_tag);

        voldesc_logical_.voldesc_seqnum = voldesc_seqnum;
        voldesc_logical_.map_table_len = 6;
        voldesc_logical_.num_part_maps = 1;
        voldesc_logical_.integrity_seq_extent = integrity_seq_extent;

        // Write parition map.
        tudf_logical_partmap_type1 part_map;
        memset(&part_map,0,sizeof(tudf_logical_partmap_type1));

        part_map.part_map_type = UDF_PARTITION_MAP_TYPE1;
        part_map.part_map_len = 6;
        part_map.volseq_num = 1;
        part_map.part_num = 0;

        // Calculate tag checksums.
        unsigned char complete_buffer[sizeof(tudf_voldesc_logical) + sizeof(tudf_logical_partmap_type1)];
        memcpy(complete_buffer,&voldesc_logical_,sizeof(tudf_voldesc_logical));
        memcpy(complete_buffer + sizeof(tudf_voldesc_logical),&part_map,sizeof(tudf_logical_partmap_type1));
        make_tag_checksums(voldesc_logical_.desc_tag,complete_buffer + sizeof(tudf_tag));

        // Write logical volume descriptor.
        out_stream.write(&voldesc_logical_,sizeof(tudf_voldesc_logical));

        // Write partition map.
        out_stream.write(&part_map,sizeof(tudf_logical_partmap_type1));

        // Pad the sector.
        char tmp[1] = { 0 };
        for (ckcore::tuint32 i = 0; i < (UDF_SECTOR_SIZE - sizeof(tudf_voldesc_logical) -
            sizeof(tudf_logical_partmap_type1)); i++)
            out_stream.write(tmp,1);
    }

    void Udf::write_vol_desc_unalloc(ckcore::CanexOutStream &out_stream,ckcore::tuint32 voldesc_seqnum,
                                     ckcore::tuint32 sec_location)
    {
        tudf_unalloc_space_desc unalloc_space_desc;
        memset(&unalloc_space_desc,0,sizeof(tudf_unalloc_space_desc));

        // Make the tag.
        make_tag(unalloc_space_desc.desc_tag,UDF_TAGIDENT_UNALLOCATEDSPACEDESC);
        unalloc_space_desc.desc_tag.tag_loc = sec_location;
        unalloc_space_desc.desc_tag.desc_crc_len = sizeof(tudf_unalloc_space_desc) - sizeof(tudf_tag);

        unalloc_space_desc.voldesc_seqnum = voldesc_seqnum;
        unalloc_space_desc.num_allocdesc = 0;

        // Calculate checksums.
        make_tag_checksums(unalloc_space_desc.desc_tag,(unsigned char *)(&unalloc_space_desc) + sizeof(tudf_tag));

        // Write to the output stream.
        out_stream.write(&unalloc_space_desc,sizeof(tudf_unalloc_space_desc));

        // Pad the sector from 512 to 2048 bytes.
        char tmp[1] = { 0 };
        for (ckcore::tuint32 i = 0; i < (UDF_SECTOR_SIZE - sizeof(tudf_unalloc_space_desc)); i++)
            out_stream.write(tmp,1);
    }

    void Udf::write_vol_desc_term(ckcore::CanexOutStream &out_stream,ckcore::tuint32 sec_location)
    {
        tudf_voldesc_term term_desc;
        memset(&term_desc,0,sizeof(tudf_voldesc_term));

        // Make the tag.
        make_tag(term_desc.desc_tag,UDF_TAGIDENT_TERMDESC);
        term_desc.desc_tag.tag_loc = sec_location;
        term_desc.desc_tag.desc_crc_len = sizeof(tudf_voldesc_term) - sizeof(tudf_tag);
        make_tag_checksums(term_desc.desc_tag,(unsigned char *)(&term_desc) + sizeof(tudf_tag));

        // Write to the output stream.
        out_stream.write(&term_desc,sizeof(tudf_voldesc_term));

        // Pad the sector from 512 to 2048 bytes.
        char tmp[1] = { 0 };
        for (ckcore::tuint32 i = 0; i < (UDF_SECTOR_SIZE - sizeof(tudf_voldesc_term)); i++)
            out_stream.write(tmp,1);
    }

    /**
        @param file_count the number of files in the file system not including
        extended attribute records.
        @param dir_count the number of directories in the file system not
        including the root directory.
        @param unique_ident must be larger than the unique udentifiers of any
        file entry.
    */
    void Udf::write_vol_desc_log_integrity(ckcore::CanexOutStream &out_stream,
                                           ckcore::tuint32 sec_location,
                                           ckcore::tuint32 file_count,ckcore::tuint32 dir_count,
                                           ckcore::tuint32 part_len,ckcore::tuint64 unique_ident,
                                           struct tm &create_time)
    {
        tudf_voldesc_logical_integrity vli;
        memset(&vli,0,sizeof(tudf_voldesc_logical_integrity));

        // Make the tag.
        make_tag(vli.desc_tag,UDF_TAGIDENT_LOGICALVOLINTEGRITYDESC);
        vli.desc_tag.tag_loc = sec_location;
        vli.desc_tag.desc_crc_len = sizeof(tudf_voldesc_logical_integrity) - sizeof(tudf_tag);

        make_date_time(create_time,vli.rec_timestamp);

        vli.integrity_type = UDF_LOGICAL_INTEGRITY_CLOSE;
        vli.num_partitions = 1;
        vli.impl_use_len = sizeof(tudf_voldesc_logical_integrity_impl_use);
        vli.free_space_table = 0;       // No free space available on the partition.
        vli.size_table = part_len;

        // Must be larger than the unique udentifiers of any file entry.
        vli.logical_volcontents_use.unique_ident = unique_ident;

        make_ident(vli.impl_use.impl_ident,IT_DEVELOPER);
        vli.impl_use.num_files = file_count;
        vli.impl_use.num_dirs = dir_count;
        vli.impl_use.min_udf_rev_read = 0x0102;     // Currently only UDF 1.02 is supported.
        vli.impl_use.min_udf_rev_write = 0x0102;    // Currently only UDF 1.02 is supported.
        vli.impl_use.max_udf_rev_write = 0x0102;    // Currently only UDF 1.02 is supported.

        // Calculate tag checksums.
        make_tag_checksums(vli.desc_tag,(unsigned char *)(&vli) + sizeof(tudf_tag));

        // Write to the output stream.
        out_stream.write(&vli,sizeof(tudf_voldesc_logical_integrity));

        // Pad the sector to 2048 bytes.
        char tmp[1] = { 0 };
        for (ckcore::tuint32 i = 0; i < (UDF_SECTOR_SIZE - sizeof(tudf_voldesc_logical_integrity)); i++)
            out_stream.write(tmp,1);
    }

    void Udf::write_anchor_vol_desc_ptr(ckcore::CanexOutStream &out_stream,
                                        ckcore::tuint32 sec_location,
                                        tudf_extent_ad &voldesc_main_seqextent,
                                        tudf_extent_ad &voldesc_rsrv_seqextent)
    {
        tudf_voldesc_anchor_ptr vap;
        memset(&vap,0,sizeof(tudf_voldesc_anchor_ptr));

        // Setup tag.
        make_tag(vap.desc_tag,UDF_TAGIDENT_ANCHORVOLDESCPTR);
        vap.desc_tag.tag_loc = sec_location;
        vap.desc_tag.desc_crc_len = sizeof(tudf_voldesc_anchor_ptr) - sizeof(tudf_tag);

        // Other members.
        memcpy(&vap.voldesc_main_seqextent,&voldesc_main_seqextent,sizeof(tudf_extent_ad));
        memcpy(&vap.voldesc_rsrv_seqextent,&voldesc_rsrv_seqextent,sizeof(tudf_extent_ad));

        // Calculate tag checksums.
        make_tag_checksums(vap.desc_tag,(unsigned char *)(&vap) + sizeof(tudf_tag));

        out_stream.write(&vap,sizeof(tudf_voldesc_anchor_ptr));

        // Pad the sector to 2048 bytes.
        char tmp[1] = { 0 };
        for (ckcore::tuint32 i = 0; i < (UDF_SECTOR_SIZE - sizeof(tudf_voldesc_anchor_ptr)); i++)
            out_stream.write(tmp,1);
    }

    /**
        Writes a file set decsriptor structure to the output stream.
        @param sec_location sector position relative to the first logical block of the partition.
    */
    void Udf::write_file_set_desc(ckcore::CanexOutStream &out_stream,ckcore::tuint32 sec_location,
                                  ckcore::tuint32 root_sec_loc,struct tm &create_time)
    {
        tudf_fileset_desc fd;
        memset(&fd,0,sizeof(tudf_fileset_desc));

        // Setup tag.
        make_tag(fd.desc_tag,UDF_TAGIDENT_FILESETDESC);
        fd.desc_tag.tag_loc = sec_location;
        fd.desc_tag.desc_crc_len = sizeof(tudf_fileset_desc) - sizeof(tudf_tag);

        make_date_time(create_time,fd.rec_timestamp);

        fd.interchange_level = UDF_INTERCHANGE_LEVEL_FILESET;
        fd.max_interchange_level = UDF_INTERCHANGE_LEVEL_FILESET;
        fd.charset_list = 1;
        fd.max_charset_list = 1;
        fd.fileset_num = 0;
        fd.fileset_descnum = 0;

        make_char_spec(fd.logical_vol_ident_charset);
        make_char_spec(fd.fileset_charset);
        memcpy(fd.logical_vol_ident,voldesc_logical_.logical_vol_ident,
            sizeof(fd.logical_vol_ident));  // Steal the value from the logical descriptor.
        memcpy(fd.fileset_ident,voldesc_primary_.vol_ident,
            sizeof(fd.fileset_ident));  // Steal the value from the primary descriptor.

        fd.rootdir_icb.extent_len = UDF_SECTOR_SIZE;
        fd.rootdir_icb.extent_loc.logical_block_num = root_sec_loc;
        fd.rootdir_icb.extent_loc.partition_ref_num = 0;    // Wee only support one partition.

        make_ident(fd.domain_ident,IT_DOMAIN);

        // Calculate tag checksums.
        make_tag_checksums(fd.desc_tag,(unsigned char *)(&fd) + sizeof(tudf_tag));

        out_stream.write(&fd,sizeof(tudf_fileset_desc));

        // Pad the sector to 2048 bytes.
        char tmp[1] = { 0 };
        for (ckcore::tuint32 i = 0; i < (UDF_SECTOR_SIZE - sizeof(tudf_voldesc_anchor_ptr)); i++)
            out_stream.write(tmp,1);
    }

    /*
        Note: This function does not pad to closest sector.
    */
    void Udf::write_file_ident_parent(ckcore::CanexOutStream &out_stream,ckcore::tuint32 sec_location,
                                      ckcore::tuint32 file_entry_sec_loc)
    {
        tudf_fileident_desc fd;
        memset(&fd,0,sizeof(tudf_fileident_desc));

        // Setup tag.
        make_tag(fd.desc_tag,UDF_TAGIDENT_FILEIDENTDESC);
        fd.desc_tag.tag_loc = sec_location;
        fd.desc_tag.desc_crc_len = sizeof(tudf_fileident_desc) + 2 - sizeof(tudf_tag);  // Always pad two bytes.

        // Setup other members.
        fd.file_ver_num = 1;
        fd.file_characteristics = UDF_FILECHARFLAG_DIRECTORY | UDF_FILECHARFLAG_PARENT;
        fd.file_ident_len = 0;

        fd.icb.extent_len = UDF_SECTOR_SIZE;
        fd.icb.extent_loc.logical_block_num = file_entry_sec_loc;
        fd.icb.extent_loc.partition_ref_num = 0;    // Always first partition.

        // Calculate tag checksums.
        unsigned char complete_buffer[sizeof(tudf_fileident_desc) + 2];
        memcpy(complete_buffer,&fd,sizeof(tudf_fileident_desc));

        // Padded bytes.
        complete_buffer[sizeof(tudf_fileident_desc)    ] = 0;
        complete_buffer[sizeof(tudf_fileident_desc) + 1] = 0;

        make_tag_checksums(fd.desc_tag,complete_buffer + sizeof(tudf_tag));

        // Re-copy the tag since the CRC and checksum has been updated.
        memcpy(complete_buffer,&fd.desc_tag,sizeof(tudf_tag));

        // Write to the output stream.
        out_stream.write(complete_buffer,sizeof(complete_buffer));
    }

    /*
        Note: This function does not pad to closest sector.
    */
    void Udf::write_file_ident(ckcore::CanexOutStream &out_stream,
                               ckcore::tuint32 sec_location,
                               ckcore::tuint32 file_entry_sec_loc,bool is_dir,
                               const ckcore::tchar *file_name)
    {
        tudf_fileident_desc fd;
        memset(&fd,0,sizeof(tudf_fileident_desc));

        // Setup tag.
        make_tag(fd.desc_tag,UDF_TAGIDENT_FILEIDENTDESC);
        fd.desc_tag.tag_loc = sec_location;

        // Setup other members.
        fd.file_ver_num = 1;
        fd.file_characteristics = is_dir ? UDF_FILECHARFLAG_DIRECTORY : 0;
        
        // Create file identifier.
        unsigned char file_ident[255];
        fd.file_ident_len = make_file_ident(file_ident,file_name);

        fd.icb.extent_len = UDF_SECTOR_SIZE;        // The file entry will always fit within one sector.
        fd.icb.extent_loc.logical_block_num = file_entry_sec_loc;
        fd.icb.extent_loc.partition_ref_num = 0;    // Always first partition.

        // Pad the file identifier.
        ckcore::tuint16 usPadSize = 4 * (ckcore::tuint16)((fd.file_ident_len +
            fd.impl_use_len + 38 + 3)/4) -
            (fd.file_ident_len + fd.impl_use_len + 38);

        // Update tag with checksums.
        ckcore::tuint16 desc_len = sizeof(tudf_fileident_desc) + fd.file_ident_len + usPadSize;
        fd.desc_tag.desc_crc_len = desc_len - sizeof(tudf_tag);

        alloc_byte_buffer(desc_len);
        memset(byte_buffer_,0,desc_len);
        memcpy(byte_buffer_,&fd,sizeof(tudf_fileident_desc));
        memcpy(byte_buffer_ + sizeof(tudf_fileident_desc),file_ident,fd.file_ident_len);

        make_tag_checksums(fd.desc_tag,byte_buffer_ + sizeof(tudf_tag));

        // Re-copy the tag since the CRC and checksum has been updated.
        memcpy(byte_buffer_,&fd.desc_tag,sizeof(tudf_tag));

        // Write to the output stream.
        out_stream.write(byte_buffer_,desc_len);
    }

    /**
        @param file_link_count the number of identifiers in the extent.
        @param unique_ident a unique identifier for this file.
        @param info_len the length of all file identifiers in bytes for
        directories and the size of the file on files.
    */
    void Udf::write_file_entry(ckcore::CanexOutStream &out_stream,ckcore::tuint32 sec_location,
                               bool is_dir,ckcore::tuint16 file_link_count,
                               ckcore::tuint64 unique_ident,ckcore::tuint32 info_loc,
                               ckcore::tuint64 info_len,struct tm &access_time,
                               struct tm &modify_time,struct tm &create_time)
    {
        tudf_file_entry fe;
        memset(&fe,0,sizeof(tudf_file_entry));

        // According to ECMA 14.14.2.2 the location must be 0 if the length is 0.
        if (info_len == 0)
            info_loc = 0;

        // Setup tag.
        make_tag(fe.desc_tag,UDF_TAGIDENT_FILEENTRYDESC);
        fe.desc_tag.tag_loc = sec_location;
        fe.desc_tag.desc_crc_len = sizeof(tudf_file_entry) - sizeof(tudf_tag);

        // Set up ICB.
        fe.icb_tag.prior_rec_num_direct_entries = 0;    // Seems to be optional.
        fe.icb_tag.strategy_type = UDF_ICB_STRATEGY_4;
        fe.icb_tag.num_entries = 1;                 // Seems to be fixed to 1.
        fe.icb_tag.file_type = is_dir ? UDF_ICB_FILETYPE_DIRECTORY :
            UDF_ICB_FILETYPE_RANDOM_BYTES;

        fe.icb_tag.parent_icb_loc.logical_block_num = 0;    // Is optional.
        fe.icb_tag.parent_icb_loc.partition_ref_num = 0;    // Is optional.

        fe.icb_tag.flags = UDF_ICB_FILEFLAG_ARCHIVE/* | UDF_ICB_FILEFLAG_LONG_ALLOC_DESC*/;
        fe.icb_tag.flags |= dvd_video_ ? UDF_ICB_FILEFLAG_SHORT_ALLOC_DESC :
            UDF_ICB_FILEFLAG_LONG_ALLOC_DESC;

        if (dvd_video_)
        {
            fe.icb_tag.flags |= UDF_ICB_FILEFLAG_NOT_RELOCATABLE | UDF_ICB_FILEFLAG_CONTIGUOUS;

            // DVD-Video does not allow files larger than 1 GiB.
            if (info_len > 0x40000000)
                throw ckcore::Exception2(ckT("DVD-Video discs does not allow files larger than 1 GiB."));
        }

        fe.uid = 0xffffffff;
        fe.gid = 0xffffffff;
        fe.permissions =
            UDF_ICB_FILEPERM_OTHER_EXECUTE | UDF_ICB_FILEPERM_OTHER_READ |
            UDF_ICB_FILEPERM_GROUP_EXECUTE | UDF_ICB_FILEPERM_GROUP_READ |
            UDF_ICB_FILEPERM_OWNER_EXECUTE | UDF_ICB_FILEPERM_OWNER_READ;

        fe.file_link_count = file_link_count;

        fe.info_len = info_len;         // flow.txt = 40, root = 264
        fe.logical_blocks_rec = bytes_to_sec64(info_len);

        // File time stamps.
        make_date_time(access_time,fe.access_time);
        make_date_time(modify_time,fe.modify_time);
        make_date_time(create_time,fe.attrib_time);

        fe.checkpoint = 1;
        make_ident(fe.impl_ident,IT_DEVELOPER);
        fe.unique_ident = unique_ident;

        // Allocation descriptor.
        tudf_long_alloc_desc lad;
        fe.allocdesc_len = 0;

        // Calculate the total number of bytes needed to store all descriptors.
        ckcore::tuint32 tot_alloc_desc_size = ((ckcore::tuint32)(info_len / 0x3ffff800) + 1) *
            sizeof(tudf_long_alloc_desc);

        // Add the extended attributes length information, and replace the
        // allocation descriptor length since DVD-Video only supports short
        // allocation descriptors.
        if (dvd_video_)
        {
            fe.extended_attr_len = sizeof(tudf_extended_attr_header_desc) +
                sizeof(tudf_extended_attr_free_ea_space) + sizeof(tudf_extended_attr_cgms);

            tot_alloc_desc_size = sizeof(tudf_short_alloc_desc);
        }

        // FIXME: Move everything to byte_buffer_.
        unsigned char *complete_buffer = new unsigned char[sizeof(tudf_file_entry) +
            fe.extended_attr_len + tot_alloc_desc_size];
        ckcore::AutoArray<unsigned char> auto_arr(complete_buffer);

        // Extended attributes that seems to be necessary for DVD-Video support.
        if (dvd_video_)
        {
            // Exended attributes header.
            tudf_extended_attr_header_desc eahd;
            memset(&eahd,0,sizeof(tudf_extended_attr_header_desc));

            // Setup the tag.
            make_tag(eahd.desc_tag,UDF_TAGIDENT_EXTENDEDATTRDESC);
            eahd.desc_tag.tag_loc = sec_location;
            eahd.desc_tag.desc_crc_len = sizeof(tudf_extended_attr_header_desc) - sizeof(tudf_tag);

            eahd.impl_attr_loc = sizeof(tudf_extended_attr_header_desc);
            eahd.app_attr_loc = sizeof(tudf_extended_attr_header_desc) +
                sizeof(tudf_extended_attr_free_ea_space) + sizeof(tudf_extended_attr_cgms);

            // Compute tag checksums.
            make_tag_checksums(eahd.desc_tag,(unsigned char *)&eahd + sizeof(tudf_tag));

            // Free EA space descriptor.
            tudf_extended_attr_free_ea_space eafes;
            memset(&eafes,0,sizeof(tudf_extended_attr_free_ea_space));

            eafes.attr_type = UDF_SECTOR_SIZE;
            eafes.attr_subtype = 1;
            eafes.attr_len = sizeof(tudf_extended_attr_free_ea_space);
            eafes.impl_use_len = 4;
            make_ident(eafes.impl_ident,IT_FREEEASPACE);
            eafes.header_checksum = make_ext_addr_checksum((unsigned char *)&eafes);
            eafes.free_space = 0;

            // CGMS descriptor.
            tudf_extended_attr_cgms eac;
            memset(&eac,0,sizeof(tudf_extended_attr_cgms));

            eac.attr_type = UDF_SECTOR_SIZE;
            eac.attr_subtype = 1;
            eac.attr_len = sizeof(tudf_extended_attr_cgms);
            eac.impl_use_len = 8;
            make_ident(eac.impl_ident,IT_CGMS);
            eac.header_checksum = make_ext_addr_checksum((unsigned char *)&eac);
            eac.cgms_info = 0;
            eac.data_struct_type = 0;
            eac.prot_sys_info = 0;

            ckcore::tuint32 buffer_pos = sizeof(tudf_file_entry);
            memcpy(complete_buffer + buffer_pos,&eahd,sizeof(tudf_extended_attr_header_desc));
            buffer_pos += sizeof(tudf_extended_attr_header_desc);

            memcpy(complete_buffer + buffer_pos,&eafes,sizeof(tudf_extended_attr_free_ea_space));
            buffer_pos += sizeof(tudf_extended_attr_free_ea_space);

            memcpy(complete_buffer + buffer_pos,&eac,sizeof(tudf_extended_attr_cgms));
            buffer_pos += sizeof(tudf_extended_attr_cgms);

            // Allocation descriptor.
            tudf_short_alloc_desc sad;
            memset(&sad,0,sizeof(tudf_short_alloc_desc));

            sad.extent_len = (ckcore::tuint32)info_len;
            sad.extent_loc = info_loc;

            fe.allocdesc_len = sizeof(tudf_short_alloc_desc);
            memcpy(complete_buffer + buffer_pos,&sad,sizeof(tudf_short_alloc_desc));
        }
        else
        {
            ckcore::tuint32 buffer_pos = sizeof(tudf_file_entry) + fe.extended_attr_len;
            while (info_len > 0x3ffff800)
            {
                memset(&lad,0,sizeof(tudf_long_alloc_desc));
                lad.extent_len = 0x3ffff800;        // FIXME: Maybe I should not state that the memory has been recorded (ECMA 14.14.1.1).
                lad.extent_loc.logical_block_num = info_loc;
                lad.extent_loc.partition_ref_num = 0;

                fe.allocdesc_len += sizeof(tudf_long_alloc_desc);

                // Copy the entry to the complete buffer.
                memcpy(complete_buffer + buffer_pos,&lad,sizeof(tudf_long_alloc_desc));
                buffer_pos += sizeof(tudf_long_alloc_desc);

                info_len -= 0x3ffff800;
                info_loc += 0x3ffff800 / UDF_SECTOR_SIZE;
            }

            // Add a descriptor containing the remaining bytes.
            memset(&lad,0,sizeof(tudf_long_alloc_desc));
            lad.extent_len = (ckcore::tuint32)info_len; // FIXME: Same as above.
            lad.extent_loc.logical_block_num = info_loc;
            lad.extent_loc.partition_ref_num = 0;

            fe.allocdesc_len += sizeof(tudf_long_alloc_desc);

            // Copy the entry to the complete buffer.
            memcpy(complete_buffer + buffer_pos,&lad,sizeof(tudf_long_alloc_desc));
        }

        // Calculate checksums.
        ckcore::tuint16 desc_len = sizeof(tudf_file_entry) +
            (ckcore::tuint16)fe.extended_attr_len +
            (ckcore::tuint16)fe.allocdesc_len;
        fe.desc_tag.desc_crc_len = desc_len - sizeof(tudf_tag);

        // The file entry part is done, copy it.
        memcpy(complete_buffer,&fe,sizeof(tudf_file_entry));

        make_tag_checksums(fe.desc_tag,complete_buffer + sizeof(tudf_tag));

        // Re-copy the tag since the CRC and checksum has been updated.
        memcpy(complete_buffer,&fe.desc_tag,sizeof(tudf_tag));

        // Write to the output stream.
        out_stream.write(complete_buffer,desc_len);

        // Pad the sector to 2048 bytes.
        char tmp[1] = { 0 };
        for (ckcore::tuint32 i = 0; i < (ckcore::tuint32)(UDF_SECTOR_SIZE - desc_len); i++)
            out_stream.write(tmp,1);
    }

    ckcore::tuint32 Udf::calc_file_ident_parent_size()
    {
        return sizeof(tudf_fileident_desc) + 2;
    }

    ckcore::tuint32 Udf::calc_file_ident_size(const ckcore::tchar *file_name)
    {
        ckcore::tuint32 file_name_len = (ckcore::tuint32)ckcore::string::astrlen(file_name);
        if (file_name_len > 254)
            file_name_len = 254;

        ckcore::tuint32 file_ident_len = dvd_video_ ? file_name_len + 1 : (file_name_len << 1) + 1;
        ckcore::tuint32 file_impl_use_len = 0;

        ckcore::tuint32 pad_size = 4 * (ckcore::tuint16)((file_ident_len + file_impl_use_len + 38 + 3)/4) -
            (file_ident_len + file_impl_use_len + 38);

        return sizeof(tudf_fileident_desc) + file_ident_len + file_impl_use_len + pad_size;
    }

    ckcore::tuint32 Udf::calc_file_entry_size()
    {
        return UDF_SECTOR_SIZE;
    }

    /**
        Returns the number of bytes needed for the initial volume recognition
        sequence.
    */
    ckcore::tuint32 Udf::get_vol_desc_initial_size()
    {
        return sizeof(tudf_volstruct_desc) * 3;
    }
};
