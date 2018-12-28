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
#include <ckcore/crcstream.hh>
#include <ckcore/canexstream.hh>

#define UDF_SECTOR_SIZE                         2048
#define UDF_UNIQUEIDENT_MIN                     16

// Tag identifiers.
#define UDF_TAGIDENT_PRIMVOLDESC                1
#define UDF_TAGIDENT_ANCHORVOLDESCPTR           2
#define UDF_TAGIDENT_VOLDESCPTR                 3
#define UDF_TAGIDENT_IMPLUSEVOLDESC             4
#define UDF_TAGIDENT_PARTDESC                   5
#define UDF_TAGIDENT_LOGICALVOLDESC             6
#define UDF_TAGIDENT_UNALLOCATEDSPACEDESC       7
#define UDF_TAGIDENT_TERMDESC                   8
#define UDF_TAGIDENT_LOGICALVOLINTEGRITYDESC    9
#define UDF_TAGIDENT_FILESETDESC                256
#define UDF_TAGIDENT_FILEIDENTDESC              257
#define UDF_TAGIDENT_FILEENTRYDESC              261
#define UDF_TAGIDENT_EXTENDEDATTRDESC           262

#define UDF_TAG_DESCRIPTOR_VERSION              2       // Goes hand in hand with "NSR02".

// D-string complession identifiers.
#define UDF_COMPRESSION_BYTE                    8
#define UDF_COMPRESSION_UNICODE                 16

// Operating system classes and identifiers.
#define UDF_OSCLASS_UNDEFINED                   0
#define UDF_OSCLASS_DOS                         1
#define UDF_OSCLASS_OS2                         2
#define UDF_OSCLASS_MACOS                       3
#define UDF_OSCLASS_UNIX                        4

#define UDF_OSIDENT_UNDEFINED                   0
#define UDF_OSIDENT_DOS                         0
#define UDF_OSIDENT_OS2                         0
#define UDF_OSIDENT_MACOS                       0
#define UDF_OSIDENT_UNIX_GENERIC                0
#define UDF_OSIDENT_UNIX_AIX                    1
#define UDF_OSIDENT_UNIX_SOLARIS                2
#define UDF_OSIDENT_UNIX_HPUX                   3
#define UDF_OSIDENT_UNIX_IRIX                   4

// Volume set interchange levels.
#define UDF_INTERCHANGE_LEVEL_MULTISET          3
#define UDF_INTERCHANGE_LEVEL_SINGLESET         2

// File set interchange levels.
#define UDF_INTERCHANGE_LEVEL_FILESET           3

// Partition flags.
#define UDF_PARTITION_FLAG_UNALLOCATED          0
#define UDF_PARTITION_FLAG_ALLOCATED            1

// Partition access types.
#define UDF_PARTITION_ACCESS_UNKNOWN            0
#define UDF_PARTITION_ACCESS_READONLY           1
#define UDF_PARTITION_ACCESS_WRITEONCE          2
#define UDF_PARTITION_ACCESS_REWRITABLE         3
#define UDF_PARTITION_ACCESS_OVERWRITABLE       4

// Partition map types.
#define UDF_PARTITION_MAP_UNKNOWN               0
#define UDF_PARTITION_MAP_TYPE1                 1
#define UDF_PARTITION_MAP_TYPE2                 2

// Domain flags.
#define UDF_DOMAIN_FLAG_HARD_WRITEPROTECT       1 << 0
#define UDF_DOMAIN_FLAG_SOFT_WRITEPROTECT       1 << 1

// Integrity types.
#define UDF_LOGICAL_INTEGRITY_OPEN              0
#define UDF_LOGICAL_INTEGRITY_CLOSE             1

// ICB strategies.
#define UDF_ICB_STRATEGY_UNKNOWN                0
#define UDF_ICB_STRATEGY_1                      1
#define UDF_ICB_STRATEGY_2                      2
#define UDF_ICB_STRATEGY_3                      3
#define UDF_ICB_STRATEGY_4                      4

// ICB file types.
#define UDF_ICB_FILETYPE_UNKNOWN                0
#define UDF_ICB_FILETYPE_UNALLOCATED_SPACE      1
#define UDF_ICB_FILETYPE_PART_INTEG_ENTRY       2
#define UDF_ICB_FILETYPE_INDIRECT_ENTRY         3
#define UDF_ICB_FILETYPE_DIRECTORY              4
#define UDF_ICB_FILETYPE_RANDOM_BYTES           5
#define UDF_ICB_FILETYPE_BLOCK_DEVICE           6
#define UDF_ICB_FILETYPE_CHARACTER_DEVICE       7
#define UDF_ICB_FILETYPE_EXTENDED_ATTR          8
#define UDF_ICB_FILETYPE_FIFO_FILE              9
#define UDF_ICB_FILETYPE_C_ISSOCK               10
#define UDF_ICB_FILETYPE_TERMINAL_ENTRY         11
#define UDF_ICB_FILETYPE_SYMBOLIC_LINK          12
#define UDF_ICB_FILETYPE_STREAM_DIRECTORY       13

// ICB file flags.
#define UDF_ICB_FILEFLAG_SHORT_ALLOC_DESC       0
#define UDF_ICB_FILEFLAG_LONG_ALLOC_DESC        1
#define UDF_ICB_FILEFLAG_EXTENDED_ALLOC_DESC    2
#define UDF_ICB_FILEFLAG_ONE_ALLOC_DESC         3
#define UDF_ICB_FILEFLAG_SORTED                 1 << 3
#define UDF_ICB_FILEFLAG_NOT_RELOCATABLE        1 << 4
#define UDF_ICB_FILEFLAG_ARCHIVE                1 << 5
#define UDF_ICB_FILEFLAG_SETUID                 1 << 6
#define UDF_ICB_FILEFLAG_SETGID                 1 << 7
#define UDF_ICB_FILEFLAG_STICKY                 1 << 8
#define UDF_ICB_FILEFLAG_CONTIGUOUS             1 << 9
#define UDF_ICB_FILEFLAG_SYSTEM                 1 << 10
#define UDF_ICB_FILEFLAG_TRANSFORMED            1 << 11
#define UDF_ICB_FILEFLAG_MULTIVERSIONS          1 << 12
#define UDF_ICB_FILEFLAG_STREAM                 1 << 13

// ICB file permissions.
#define UDF_ICB_FILEPERM_OTHER_EXECUTE          1 << 0
#define UDF_ICB_FILEPERM_OTHER_WRITE            1 << 1
#define UDF_ICB_FILEPERM_OTHER_READ             1 << 2
#define UDF_ICB_FILEPERM_OTHER_CHANGEATTRIB     1 << 3
#define UDF_ICB_FILEPERM_OTHER_DELETE           1 << 4
#define UDF_ICB_FILEPERM_GROUP_EXECUTE          1 << 5
#define UDF_ICB_FILEPERM_GROUP_WRITE            1 << 6
#define UDF_ICB_FILEPERM_GROUP_READ             1 << 7
#define UDF_ICB_FILEPERM_GROUP_CHANGEATTRIB     1 << 8
#define UDF_ICB_FILEPERM_GROUP_DELETE           1 << 9
#define UDF_ICB_FILEPERM_OWNER_EXECUTE          1 << 10
#define UDF_ICB_FILEPERM_OWNER_WRITE            1 << 11
#define UDF_ICB_FILEPERM_OWNER_READ             1 << 12
#define UDF_ICB_FILEPERM_OWNER_CHANGEATTRIB     1 << 13
#define UDF_ICB_FILEPERM_OWNER_DELETE           1 << 14

// File characterisic flags.
#define UDF_FILECHARFLAG_EXIST                  1 << 0
#define UDF_FILECHARFLAG_DIRECTORY              1 << 1
#define UDF_FILECHARFLAG_DELETED                1 << 2
#define UDF_FILECHARFLAG_PARENT                 1 << 3
#define UDF_FILECHARFLAG_METADATA               1 << 4

// Parition entity flags.
#define UDF_ENTITYFLAG_DVDVIDEO                 1 << 1

namespace ckfilesystem
{
#pragma pack(1) // Force byte alignment.

    /*
        Volume Structures.
    */
    typedef struct
    {
        unsigned char type;                 // Must be 0.
        unsigned char ident[5];             // "BEA01", "NSR03", "TEA01".
        unsigned char struct_ver;           // Must be 1.
        unsigned char struct_data[2041];
    } tudf_volstruct_desc;

    typedef struct
    {
        unsigned char charset_type;     // Must be 0.
        unsigned char charset_info[63]; // "OSTA Compressed Unicode".
    } tudf_charspec;

    typedef struct  // ISO 13346 1/7.3.
    {
        ckcore::tuint16 type_tz;
        ckcore::tuint16 year;
        unsigned char mon;
        unsigned char day;
        unsigned char hour;
        unsigned char min;
        unsigned char sec;
        unsigned char centisec;
        unsigned char hundreds_of_microsec;
        unsigned char microsec;
    } tudf_timestamp;

    typedef struct  // ISO 13346 1/7.4.
    { 
        unsigned char flags;
        unsigned char ident[23];
        unsigned char ident_suffix[8];
    } tudf_intity_ident;

    typedef struct  // ISO 13346 3/7.2.
    {
        ckcore::tuint16 tag_ident;
        ckcore::tuint16 desc_ver;
        unsigned char tag_chksum;
        unsigned char res1;
        ckcore::tuint16 tag_serial_num;
        ckcore::tuint16 desc_crc;
        ckcore::tuint16 desc_crc_len;
        ckcore::tuint32 tag_loc;
    } tudf_tag;

    /*typedef struct    // ISO 13346 4/14.5.
    {
        tudf_tag desc_tag;
        ckcore::tuint32 prev_allocextent_loc;
        ckcore::tuint32 alloc_desc_len;
    } tudf_alloc_extent_desc;*/

    typedef struct  // ISO 13346 3/7.1
    {
        ckcore::tuint32 extent_len;
        ckcore::tuint32 extent_loc;
    } tudf_extent_ad;

    typedef struct  // ISO 13346 3/10.1.
    {
        tudf_tag desc_tag;
        ckcore::tuint32 voldesc_seqnum;
        ckcore::tuint32 voldesc_primnum;
        unsigned char vol_ident[32];        // D-characters.
        ckcore::tuint16 volseq_num;
        ckcore::tuint16 max_volseq_num;
        ckcore::tuint16 interchange_level;
        ckcore::tuint16 max_interchange_level;
        ckcore::tuint32 charset_list;
        ckcore::tuint32 max_charset_list;
        unsigned char volset_ident[128];    // D-characters.
        tudf_charspec desc_charset;
        tudf_charspec explanatory_charset;
        tudf_extent_ad vol_abstract;
        tudf_extent_ad vol_copyright_notice;
        tudf_intity_ident app_ident;
        tudf_timestamp rec_timestamp;
        tudf_intity_ident impl_ident;
        unsigned char impl_use[64];
        ckcore::tuint32 predecessor_voldesc_seqloc;
        ckcore::tuint16 flags;
        unsigned char res1[22];
    } tudf_voldesc_prim;

    typedef struct
    {
        tudf_charspec lv_info_charset;
        unsigned char lv_ident[128];        // D-characters.
        unsigned char lv_info1[36];             // D-characters.
        unsigned char lv_info2[36];             // D-characters.
        unsigned char lv_info3[36];             // D-characters.
        tudf_intity_ident impl_ident;
        unsigned char impl_use[128];
    } tudf_lv_info;

    typedef struct
    {
        tudf_tag desc_tag;
        ckcore::tuint32 voldesc_seqnum;
        tudf_intity_ident impl_ident;
        tudf_lv_info lv_info;
    } tudf_voldesc_impl_use;

    typedef struct
    {
        tudf_tag desc_tag;
        ckcore::tuint32 voldesc_seqnum;
        ckcore::tuint16 part_flags;
        ckcore::tuint16 part_num;
        tudf_intity_ident part_content_ident;
        unsigned char part_content_use[128];
        ckcore::tuint32 access_type;
        ckcore::tuint32 part_start_loc;
        ckcore::tuint32 part_len;
        tudf_intity_ident impl_ident;
        unsigned char impl_use[128];
        unsigned char res1[156];
    } tudf_voldesc_part;

    typedef struct  // ISO 13346 3/10.6.
    {
        tudf_tag desc_tag;
        ckcore::tuint32 voldesc_seqnum;
        tudf_charspec desc_charset;
        unsigned char logical_vol_ident[128];   // D-characters.
        ckcore::tuint32 logical_block_size;
        tudf_intity_ident domain_ident;
        unsigned char logocal_vol_contents_use[16];
        ckcore::tuint32 map_table_len;
        ckcore::tuint32 num_part_maps;
        tudf_intity_ident impl_ident;
        unsigned char impl_use[128];
        tudf_extent_ad integrity_seq_extent;
        //unsigned char part_maps[1];           // Actually num_part_maps.
    } tudf_voldesc_logical; // No maximum size.

    typedef struct  // ISO 13346 3/17
    {
        unsigned char part_map_type;            // Always UDF_PARTITION_MAP_TYPE1.
        unsigned char part_map_len;             // Always 6.
        ckcore::tuint16 volseq_num;
        ckcore::tuint16 part_num;
    } tudf_logical_partmap_type1;

    typedef struct  // ISO 13346 3/18
    {
        unsigned char part_map_type;            // Always UDF_PARTITION_MAP_TYPE2.
        unsigned char part_map_len;             // Always 64.
        unsigned char part_ident[62];
    } tudf_logical_partmap_type2;

    typedef struct  // ISO 13346 3/10.8.
    {
        tudf_tag desc_tag;
        ckcore::tuint32 voldesc_seqnum;
        ckcore::tuint32 num_allocdesc;
        //tudf_extent_ad alloc_desc[1];         // Actually num_allocdesc.
    } tudf_unalloc_space_desc;  // No maximum size.

    typedef struct
    {
        tudf_tag desc_tag;
        unsigned char res1[496];
    } tudf_voldesc_term;

    typedef struct  // ISO 13346 4/14.15.
    {
        ckcore::tuint64 unique_ident;
        unsigned char res1[24];
    } tudf_voldesc_logical_header;

    typedef struct
    {
        tudf_intity_ident impl_ident;
        ckcore::tuint32 num_files;
        ckcore::tuint32 num_dirs;
        ckcore::tuint16 min_udf_rev_read;
        ckcore::tuint16 min_udf_rev_write;
        ckcore::tuint16 max_udf_rev_write;
    } tudf_voldesc_logical_integrity_impl_use;

    typedef struct  // ISO 13346 3/10.10.
    {
        tudf_tag desc_tag;
        tudf_timestamp rec_timestamp;
        ckcore::tuint32 integrity_type;
        tudf_extent_ad next_integrity_extent;
        tudf_voldesc_logical_header logical_volcontents_use;
        ckcore::tuint32 num_partitions;
        ckcore::tuint32 impl_use_len;
        ckcore::tuint32 free_space_table;
        ckcore::tuint32 size_table;
        tudf_voldesc_logical_integrity_impl_use impl_use;
    } tudf_voldesc_logical_integrity;

    typedef struct  // ISO 13346 3/10.2.
    {
        tudf_tag desc_tag;
        tudf_extent_ad voldesc_main_seqextent;
        tudf_extent_ad voldesc_rsrv_seqextent;
        unsigned char res1[480];
    } tudf_voldesc_anchor_ptr;      // Must be 512 bytes.

    /*
        Partition Structures.
    */
    typedef struct  // ISO 13346 4/7.1.
    {
        ckcore::tuint32 logical_block_num;
        ckcore::tuint16 partition_ref_num;
    } tudf_attr_lb;

    typedef struct  // ISO 13346 - 4/14.14.1. (Short Allocation Descriptor)
    {
        ckcore::tuint32 extent_len;
        ckcore::tuint32 extent_loc;
    } tudf_short_alloc_desc;

    typedef struct  // ISO 13346 - 4/14.14.2. (Long Allocation Descriptor)
    {
        ckcore::tuint32 extent_len;
        tudf_attr_lb extent_loc;
        unsigned char impl_use[6];
    } tudf_long_alloc_desc;

    typedef struct  // ISO 13346 4/14.1.
    {
        tudf_tag desc_tag;
        tudf_timestamp rec_timestamp;
        ckcore::tuint16 interchange_level;
        ckcore::tuint16 max_interchange_level;
        ckcore::tuint32 charset_list;
        ckcore::tuint32 max_charset_list;
        ckcore::tuint32 fileset_num;
        ckcore::tuint32 fileset_descnum;
        tudf_charspec logical_vol_ident_charset;
        unsigned char logical_vol_ident[128];   // D-characters.
        tudf_charspec fileset_charset;
        unsigned char fileset_ident[32];        // D-characters.
        unsigned char copy_file_ident[32];      // D-characters.
        unsigned char abst_file_ident[32];      // D-characters.
        tudf_long_alloc_desc rootdir_icb;
        tudf_intity_ident domain_ident;
        tudf_long_alloc_desc next_extent;
        unsigned char res1[48];
    } tudf_fileset_desc;    // Must be 512 bytes.

    typedef struct  // ISO 13346 4/14.6.
    {
        ckcore::tuint32 prior_rec_num_direct_entries;
        ckcore::tuint16 strategy_type;
        unsigned char strategy_param[2];
        ckcore::tuint16 num_entries;
        unsigned char res1;
        unsigned char file_type;
        tudf_attr_lb parent_icb_loc;
        ckcore::tuint16 flags;
    } tudf_tagicb;

    typedef struct  // ISO 13346 4/14.9.
    {
        tudf_tag desc_tag;
        tudf_tagicb icb_tag;
        ckcore::tuint32 uid;
        ckcore::tuint32 gid;
        ckcore::tuint32 permissions;
        ckcore::tuint16 file_link_count;
        unsigned char rec_format;
        unsigned char rec_disp_attr;
        ckcore::tuint32 rec_len;
        ckcore::tuint64 info_len;
        ckcore::tuint64 logical_blocks_rec;
        tudf_timestamp access_time;
        tudf_timestamp modify_time;
        tudf_timestamp attrib_time;
        ckcore::tuint32 checkpoint;
        tudf_long_alloc_desc extended_attr_icb;
        tudf_intity_ident impl_ident;
        ckcore::tuint64 unique_ident;
        ckcore::tuint32 extended_attr_len;
        ckcore::tuint32 allocdesc_len;

        //unsigned char extended_attr[extended_attr_len];
        //unsigned char allocdesc[allocdesc_len];
    } tudf_file_entry;  // Maximum of a logical block size.

    typedef struct  // ISO 13346 4/14.4.
    {
        tudf_tag desc_tag;
        ckcore::tuint16 file_ver_num;
        unsigned char file_characteristics;
        unsigned char file_ident_len;
        tudf_long_alloc_desc icb;
        ckcore::tuint16 impl_use_len;
        //unsigned char impl_use[1];            // Actually impl_use_len.
        //char file_ident[1];                   // Actually file_ident_len.
        //unsigned char padding[...];
    } tudf_fileident_desc;  // Maximum of a logical block size.

    /*
        Extended attributes.
    */
    typedef struct  // ISO 13346 4/14.10.1.
    {
        tudf_tag desc_tag;
        ckcore::tuint32 impl_attr_loc;
        ckcore::tuint32 app_attr_loc;
    } tudf_extended_attr_header_desc;

    /*typedef struct    // ISO 13346 4/14.10.8.
    {
        ckcore::tuint32 attr_type;
        unsigned char attr_subtype;
        unsigned char res1[3];
        ckcore::tuint32 attr_len;
        ckcore::tuint32 impl_use_len;
        tudf_intity_ident impl_ident;
        //unsigned char impl_use[impl_use_len];
    } tudf_impl_use_extended_attr;*/

    typedef struct  // UDF 1.02 - 3.3.4.5.1.1
    {
        ckcore::tuint32 attr_type;
        unsigned char attr_subtype;
        unsigned char res1[3];
        ckcore::tuint32 attr_len;
        ckcore::tuint32 impl_use_len;
        tudf_intity_ident impl_ident;
        ckcore::tuint16 header_checksum;
        ckcore::tuint16 free_space;
    } tudf_extended_attr_free_ea_space;

    typedef struct  // UDF 1.02 - 3.3.4.5.1.2
    {
        ckcore::tuint32 attr_type;
        unsigned char attr_subtype;
        unsigned char res1[3];
        ckcore::tuint32 attr_len;
        ckcore::tuint32 impl_use_len;
        tudf_intity_ident impl_ident;
        ckcore::tuint16 header_checksum;
        unsigned char cgms_info;
        unsigned char data_struct_type;
        ckcore::tuint32 prot_sys_info;
    } tudf_extended_attr_cgms;

    /*typedef struct    // ISO 13346 4/14.3.
    {
        tudf_short_alloc_desc unalloc_space_table;
        tudf_short_alloc_desc unalloc_space_bitmap;
        tudf_short_alloc_desc partition_integrity_table;
        tudf_short_alloc_desc freed_space_table;
        tudf_short_alloc_desc freed_space_bitmap;
        unsigned char res1[88];
    } tudf_part_header_desc;

    typedef struct  // ISO 13346 4/14.11.
    {
        tudf_tag desc_tag;
        tudf_tagICB icb_tag;
        ckcore::tuint32 allocdesc_len;
        //unsigned char allocdesc[allocdesc_len];
    } tudf_unallocd_space_entry;    // Maximum of a logical block size.

    typedef struct  // ISO 13346 4/14.11.
    {
        tudf_tag desc_tag;
        ckcore::tuint32 num_bits;
        ckcore::tuint32 num_bytes;
        //unsigned char bitmap[num_bytes];
    } tudf_space_bitmap;    // No maximum size.

    typedef struct  // ISO 13346 4/14.13.
    {
        tudf_tag desc_tag;
        tudf_tagICB icb_tag;
        tudf_timestamp rec_timestamp;
        unsigned char integrity_type;
        unsigned char res1[175];
        tudf_intity_ident impl_ident;
        unsigned char impl_use[256];
    } tudf_part_integrity_entry;

    typedef struct  // ISO 13346 4/14.16.1.
    {
        unsigned char comp_type;
        unsigned char comp_ident_len;
        ckcore::tuint16 comp_file_ver_num;
        //char comp_ident[comp_ident_len];
    } tudf_path_comp;*/

    /*typedef struct    // ISO 13346 4/14.10.4.
    {
        ckcore::tuint32 attr_type;
        unsigned char attr_subtype;
        unsigned char res1[3];
        ckcore::tuint32 attr_len;
        ckcore::tuint16 owner_ident;
        ckcore::tuint16 group_ident;
        ckcore::tuint16 permission;
    } tudf_alt_permissions_extended_attr;

    typedef struct  // ISO 13346 4/14.10.5.
    {
        ckcore::tuint32 attr_type;
        unsigned char attr_subtype;
        unsigned char res1[3];
        ckcore::tuint32 attr_len;
        ckcore::tuint32 data_len;
        ckcore::tuint32 file_time_existence;
        unsigned char file_times;
    } tudf_filetimes_extended_attr;

    typedef struct  // ISO 13346 4/14.10.7.
    {
        ckcore::tuint32 attr_type;
        unsigned char attr_subtype;
        unsigned char res1[3];
        ckcore::tuint32 attr_len;
        ckcore::tuint32 impl_use_len;
        ckcore::tuint32 major_dev_ident;
        ckcore::tuint32 minor_dev_ident;
        //unsigned char impl_use[impl_use_len];
    } tudf_devspec_extendedattr;

    typedef struct  // ISO 13346 4/14.10.9.
    {
        ckcore::tuint32 attr_type;
        unsigned char attr_subtype;
        unsigned char res1[3];
        ckcore::tuint32 attr_len;
        ckcore::tuint32 app_use_len;
        tudf_intity_ident app_ident;
        //unsigned char app_use[app_use_len];
    } tudf_app_use_extended_attr;*/

#pragma pack()  // Switch back to normal alignment.

    class Udf
    {
    public:
        enum PartAccessType
        {
            AT_UNKNOWN,
            AT_READONLY,
            AT_WRITEONCE,
            AT_REWRITABLE,
            AT_OVERWRITABLE
        };

    private:
        // Enumartion of different descriptor types.
        enum IdentType
        {
            IT_DEVELOPER,
            IT_LVINFO,
            IT_DOMAIN,
            IT_FREEEASPACE,
            IT_CGMS
        };

        tudf_voldesc_prim voldesc_primary_;
        tudf_voldesc_part voldesc_partition_;
        tudf_voldesc_logical voldesc_logical_;

        ckcore::CrcStream crc_stream_;

        // Determines what access will be given to the parition.
        PartAccessType part_access_type_;

        // Set to true of writing a DVD-Video compatible file system.
        bool dvd_video_;

        // Buffer used for various data storage. This is used for performance reasons.
        unsigned char *byte_buffer_;
        ckcore::tuint32 byte_buffer_size_;

        void alloc_byte_buffer(ckcore::tuint32 min_size);

        size_t compress_unicode_str(size_t num_chars,unsigned char comp_id,
                                    const wchar_t *in_str,unsigned char *out_str);

        void init_vol_desc_primary();
        void init_vol_desc_partition();
        void init_vol_desc_logical();

        void make_char_spec(tudf_charspec &char_spec);
        void make_ident(tudf_intity_ident &impl_ident,IdentType ident_type);
        void make_tag(tudf_tag &tag,ckcore::tuint16 ident);
        void make_tag_checksums(tudf_tag &tag,unsigned char *buffer);
        void make_vol_set_ident(unsigned char *volset_ident,size_t volset_ident_size);
        void make_date_time(struct tm &time,tudf_timestamp &udf_time);
        void make_os_identifiers(unsigned char &os_class,unsigned char &os_ident);

        unsigned char make_file_ident(unsigned char *out_buffer,const ckcore::tchar *file_name);

        ckcore::tuint16 make_ext_addr_checksum(unsigned char *buffer);

    public:
        Udf(bool dvd_video);
        ~Udf();

        // Change of internal state functions.
        void set_volume_label(const ckcore::tchar *label);
        void set_part_access_type(PartAccessType access_type);

        // Write functions.
        void write_vol_desc_initial(ckcore::CanexOutStream &out_stream);
        void write_vol_desc_primary(ckcore::CanexOutStream &out_stream,ckcore::tuint32 voldesc_seqnum,
                                    ckcore::tuint32 sec_location,struct tm &create_time);
        void write_vol_desc_impl_use(ckcore::CanexOutStream &out_stream,ckcore::tuint32 voldesc_seqnum,
                                     ckcore::tuint32 sec_location);
        void write_vol_desc_partition(ckcore::CanexOutStream &out_stream,ckcore::tuint32 voldesc_seqnum,
                                      ckcore::tuint32 sec_location,ckcore::tuint32 part_start_loc,
                                      ckcore::tuint32 part_len);
        void write_vol_desc_logical(ckcore::CanexOutStream &out_stream,ckcore::tuint32 voldesc_seqnum,
                                    ckcore::tuint32 sec_location,tudf_extent_ad &integrity_seq_extent);
        void write_vol_desc_unalloc(ckcore::CanexOutStream &out_stream,ckcore::tuint32 voldesc_seqnum,
                                    ckcore::tuint32 sec_location);
        void write_vol_desc_term(ckcore::CanexOutStream &out_stream,ckcore::tuint32 sec_location);
        void write_vol_desc_log_integrity(ckcore::CanexOutStream &out_stream,ckcore::tuint32 sec_location,
                                          ckcore::tuint32 file_count,ckcore::tuint32 dir_count,
                                          ckcore::tuint32 part_len,ckcore::tuint64 unique_ident,
                                          struct tm &create_time);
        void write_anchor_vol_desc_ptr(ckcore::CanexOutStream &out_stream,ckcore::tuint32 sec_location,
                                       tudf_extent_ad &voldesc_main_seqextent,
                                       tudf_extent_ad &voldesc_rsrv_seqextent);

        void write_file_set_desc(ckcore::CanexOutStream &out_stream,ckcore::tuint32 sec_location,
                                 ckcore::tuint32 root_sec_loc,struct tm &create_time);
        void write_file_ident_parent(ckcore::CanexOutStream &out_stream,ckcore::tuint32 sec_location,
                                     ckcore::tuint32 file_entry_sec_loc);
        void write_file_ident(ckcore::CanexOutStream &out_stream,ckcore::tuint32 sec_location,
                              ckcore::tuint32 file_entry_sec_loc,bool is_dir,
                              const ckcore::tchar *file_name);
        void write_file_entry(ckcore::CanexOutStream &out_stream,ckcore::tuint32 sec_location,
                              bool is_dir,ckcore::tuint16 file_link_count,
                              ckcore::tuint64 unique_ident,ckcore::tuint32 info_loc,
                              ckcore::tuint64 info_len,struct tm &access_time,
                              struct tm &modify_time,struct tm &create_time);

        // Helper functions.
        ckcore::tuint32 calc_file_ident_parent_size();
        ckcore::tuint32 calc_file_ident_size(const ckcore::tchar *file_name);
        ckcore::tuint32 calc_file_entry_size();

        ckcore::tuint32 get_vol_desc_initial_size();
    };
};

