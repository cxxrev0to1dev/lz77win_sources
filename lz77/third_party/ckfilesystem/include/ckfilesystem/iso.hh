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

#define ISO_SECTOR_SIZE                      2048
#define ISO_MAX_NAMELEN_1999                  207
#define ISO_MAX_DIRLEVEL_NORMAL                 8           // Maximum is 8 for ISO9660:1988.
#define ISO_MAX_DIRLEVEL_1999                 255           // Maximum is 255 for ISO9660:1999.
#define ISO_MAX_EXTENT_SIZE                  0xfffff800

#define DIRRECORD_FILEFLAG_HIDDEN               1 << 0
#define DIRRECORD_FILEFLAG_DIRECTORY            1 << 1
#define DIRRECORD_FILEFLAG_ASSOCIATEDFILE       1 << 2
#define DIRRECORD_FILEFLAG_RECORD               1 << 3
#define DIRRECORD_FILEFLAG_PROTECTION           1 << 4
#define DIRRECORD_FILEFLAG_MULTIEXTENT          1 << 7

#define VOLDESCTYPE_BOOT_CATALOG                0
#define VOLDESCTYPE_PRIM_VOL_DESC               1
#define VOLDESCTYPE_SUPPL_VOL_DESC              2
#define VOLDESCTYPE_VOL_PARTITION_DESC          3
#define VOLDESCTYPE_VOL_DESC_SET_TERM           255

namespace ckfilesystem
{
    /*
        Identifiers.
    */
    extern const char *iso_ident_cd;
    extern const char *iso_ident_eltorito;

    /*
        File and Directory Descriptors.
    */
#pragma pack(1) // Force byte alignment.

    typedef struct
    {
        unsigned char year;     // Number of years since 1900.
        unsigned char mon;      // Month of the year from 1 to 12.
        unsigned char day;      // Day of the month from 1 to 31.
        unsigned char hour;     // Hour of the day from 0 to 23.
        unsigned char min;      // Minute of the hour from 0 to 59.
        unsigned char sec;      // Second of the minute from 0 to 59.
        unsigned char zone;     // Offset from Greenwich Mean Time in number of
                                // 15 min intervals from -48 (West) to + 52 (East)
                                // recorded according to 7.1.2.
    } tiso_dir_record_datetime;

    typedef struct
    {
        unsigned char dir_record_len;
        unsigned char ext_attr_record_len;
        unsigned char extent_loc[8];    // 7.3.3.
        unsigned char data_len[8];      // 7.3.3.
        tiso_dir_record_datetime rec_timestamp;
        unsigned char file_flags;
        unsigned char file_unit_size;
        unsigned char interleave_gap_size;
        unsigned char volseq_num[4];    // 7.2.3.
        unsigned char file_ident_len;
        unsigned char file_ident[1];    // Actually of size file_ident_len.
    } tiso_dir_record;

    typedef struct
    {
        unsigned char dir_ident_len;
        unsigned char ext_attr_record_len;
        unsigned char extent_loc[4];        // 7.3.?.
        unsigned char parent_dir_num[2];    // 7.2.?.
        unsigned char dir_ident[1];         // Actually consumes the rest of the
                                            // available path table record size.
    } tiso_pathtable_record;

    typedef struct
    {
        unsigned char owner_ident[4];       // 7.2.3.
        unsigned char group_ident[4];       // 7.2.3.
        ckcore::tuint16 permissions;
        tiso_dir_record_datetime create_time;
        tiso_dir_record_datetime modify_time;
        tiso_dir_record_datetime expr_time;
        tiso_dir_record_datetime effect_time;
        unsigned char rec_format;
        unsigned char rec_attr;
        unsigned char rec_len[4];           // 7.2.3.
        unsigned char sys_ident[32];
        unsigned char sys_data[64];
        unsigned char ext_attr_record_ver;
        unsigned char esc_len;
        unsigned char res1[64];
        unsigned char app_data_len[4];      // 7.2.3.
        unsigned char app_data[1];          // Actually of size uiAppDataLen.
    } tiso_ext_attr_record;

    /*
        Volume Descriptors.
    */
    typedef struct
    {
        ckcore::tuint32 year;       // Year from 1 to 9999.
        ckcore::tuint16 mon;        // Month of the year from 1 to 12.
        ckcore::tuint16 day;        // Day of the month from 1 to 31.
        ckcore::tuint16 hour;       // Hour of the day from 0 to 23.
        ckcore::tuint16 min;        // Minute of the hour from 0 to 59.
        ckcore::tuint16 sec;        // Second of the minute from 0 to 59.
        ckcore::tuint16 hundreds;   // Hundredths of a second.
        unsigned char zone;         // Offset from Greenwich Mean Time in number of
                                    // 15 min intervals from -48 (West) to +52 (East)
                                    // recorded according to 7.1.2.
    } tiso_voldesc_datetime;

    typedef struct
    {
        unsigned char type;         // 0.
        unsigned char ident[5];     // "CD001".
        unsigned char version;
        unsigned char boot_sys_ident[32];
        unsigned char boot_ident[32];
        unsigned char boot_sys_data[1977];
    } tiso_voldesc_bootrec;         // Must be 2048 bytes in size.

    typedef struct
    {
        unsigned char type;                 // 0.
        unsigned char ident[5];             // "CD001".
        unsigned char version;              // Must be 1.
        unsigned char boot_sys_ident[32];   // Must be "EL TORITO SPECIFICATION" padded with 0s.
        unsigned char unused1[32];          // Must be 0.
        unsigned int boot_cat_ptr;          // Absolute pointer to first sector of Boot Catalog.
        unsigned char boot_sys_data[1973];
    } tiso_voldesc_eltorito_record; // Must be 2048 bytes in size.

    typedef struct
    {
        unsigned char type;
        unsigned char ident[5];                 // "CD001".
        unsigned char version;
        unsigned char unused1;
        unsigned char sys_ident[32];
        unsigned char vol_ident[32];
        unsigned char unused2[8];
        unsigned char vol_space_size[8];        // 7.3.3.
        unsigned char unused3[32];
        unsigned char volset_size[4];           // 7.2.3.
        unsigned char volseq_num[4];            // 7.2.3.
        unsigned char logical_block_size[4];    // 7.2.3.
        unsigned char path_table_size[8];       // 7.3.3.
        unsigned char path_table_type_l[4];     // 7.3.1.
        unsigned char opt_path_table_type_l[4]; // 7.3.1.
        unsigned char path_table_type_m[4];     // 7.3.2.
        unsigned char opt_path_table_type_m[4]; // 7.3.2.
        tiso_dir_record root_dir_record;
        unsigned char volset_ident[128];
        unsigned char publ_ident[128];
        unsigned char prep_ident[128];
        unsigned char app_ident[128];
        unsigned char copy_file_ident[37];
        unsigned char abst_file_ident[37];
        unsigned char bibl_file_ident[37];
        tiso_voldesc_datetime create_time;
        tiso_voldesc_datetime modify_time;
        tiso_voldesc_datetime expr_time;
        tiso_voldesc_datetime effect_time;
        unsigned char file_struct_ver;
        unsigned char unused4;
        unsigned char app_data[512];
        unsigned char unused5[653];
    } tiso_voldesc_primary;     // Must be 2048 bytes in size.

    typedef struct
    {
        unsigned char type;
        unsigned char ident[5];                 // "CD001".
        unsigned char version;
        unsigned char vol_flags;
        unsigned char sys_ident[32];
        unsigned char vol_ident[32];
        unsigned char unused1[8];
        unsigned char vol_space_size[8];        // 7.3.3.
        unsigned char esc_sec[32];
        unsigned char volset_size[4];           // 7.2.3.
        unsigned char volseq_num[4];            // 7.2.3.
        unsigned char logical_block_size[4];    // 7.2.3.
        unsigned char path_table_size[8];       // 7.3.3.
        unsigned char path_table_type_l[4];     // 7.3.1.
        unsigned char opt_path_table_type_l[4]; // 7.3.1.
        unsigned char path_table_type_m[4];     // 7.3.2.
        unsigned char opt_path_table_type_m[4]; // 7.3.2.
        tiso_dir_record root_dir_record;
        unsigned char volset_ident[128];
        unsigned char publ_ident[128];
        unsigned char prep_ident[128];
        unsigned char app_ident[128];
        unsigned char copy_file_ident[37];
        unsigned char abst_file_ident[37];
        unsigned char bibl_file_ident[37];
        tiso_voldesc_datetime create_time;
        tiso_voldesc_datetime modify_time;
        tiso_voldesc_datetime expr_time;
        tiso_voldesc_datetime effect_time;
        unsigned char file_struct_ver;
        unsigned char unused2;
        unsigned char app_data[512];
        unsigned char unused3[653];
    } tiso_voldesc_suppl;       // Must be 2048 bytes in size.

    typedef struct
    {
        unsigned char type;             // 3.
        unsigned char ident[5];         // "CD001".
        unsigned char version;
        unsigned char unused1;
        unsigned char sys_ident[32];
        unsigned char part_ident[32];
        unsigned char part_loc[8];      // 7.3.3.
        unsigned char part_size[8];     // 7.3.3.
        unsigned char sys_data[1960];
    } tiso_voldesc_part;    // Must be 2048 bytes in size.

    typedef struct
    {
        unsigned char type;     // 255.
        unsigned char ident[5]; // "CD001".
        unsigned char version;
        unsigned char res1[2041];
    } tiso_voldesc_setterm;     // Must be 2048 bytes in size.

#pragma pack()  // Switch back to normal alignment.

    /**
     * @brief Supported character sets.
     */
    enum CharacterSet
    {
        CHARSET_ISO,    ///< Character set as specified in the ISO9660 standard.
        CHARSET_DOS,    ///< DOS (code page 437) character set (256 characters).
        CHARSET_ASCII   ///< ASCII character set (128 characters).
    };

    /**
     * Converts an ASCII character to an a-character (appendix A).
     * @param [in] c Character to convert.
     * @param [in] char_set Character set to use.
     * @return c converted to an a-character.
     */
    char iso_make_char_a(char c, CharacterSet char_set);

    /**
     * Converts an ASCII character to a d-character (appendix A).
     * @param [in] c Character to convert.
     * @param [in] char_set Character set to use.
     * @return c converted to a d-character.
     */
    char iso_make_char_d(char c, CharacterSet char_set);

    /**
     * Performs a memory copy from source to target, all characters
     * in target will be converted to a-characters.
     * @param [out] dst Destination memory buffer.
     * @param [in] src Source memory buffer.
     * @param [in] size Number of bytes to copy.
     * @param [in] char_set Character set to use.
     */
    void iso_memcpy_a(unsigned char *dst, const char *src, size_t size, CharacterSet char_set);

    /**
     * Performs a memory copy from source to target, all characters
     * in target will be converted to d-characters.
     * @param [out] dst Destination memory buffer.
     * @param [in] src Source memory buffer.
     * @param [in] size Number of bytes to copy.
     * @param [in] char_set Character set to use.
     */
    void iso_memcpy_d(unsigned char *dst, const char *src, size_t size, CharacterSet char_set);

    /**
     * Finds the last specified delimiter in the specified string of
     * a-characters.
     * @param [in] str String to find delimiter in.
     * @param [in] delim Delimiter to find.
     * @return If successful the index of the first matching delimiter, if
     *         unsuccessful -1 is returned.
     */
    int iso_last_delimiter_a(const char *str, char delim);

    /**
     * Converts the input file name to a valid ISO level 1 file name. This
     * means:
     *  - A maximum of 12 characters.
     *  - A file extension of at most 3 characters.
     *  - A file name of at most 8 characters.
     * @param [out] buffer Output buffer to write file name to.
     * @param [in] file_name File name to write.
     * @param [in] char_set Character set to use.
     * @return Length of output string.
     */
    unsigned char iso_write_file_name_l1(unsigned char *buffer, const ckcore::tchar *file_name,
                                         CharacterSet char_set);

    /**
     * Converts the input file name to a valid ISO level 2 and above file name.
     * This means:
     *  - A maximum of 31 characters.
     */
    unsigned char iso_write_file_name_l2(unsigned char *buffer ,const ckcore::tchar *file_name,
                                         CharacterSet char_set);

    unsigned char iso_write_file_name_1999(unsigned char *buffer, const ckcore::tchar *file_name,
                                           CharacterSet char_set);
    unsigned char iso_write_file_name_generic(unsigned char *buffer, const ckcore::tchar *file_name,
                                              int max_len, CharacterSet char_set);

    unsigned char iso_write_dir_name_l1(unsigned char *buffer, const ckcore::tchar *dir_name,
                                        CharacterSet char_set);
    unsigned char iso_write_dir_name_l2(unsigned char *buffer, const ckcore::tchar *dir_name,
                                        CharacterSet char_set);
    unsigned char iso_write_dir_name_1999(unsigned char *buffer, const ckcore::tchar *dir_name,
                                          CharacterSet char_set);
    unsigned char iso_write_dir_name_generic(unsigned char *buffer, const ckcore::tchar *dir_name,
                                             int max_len, CharacterSet char_set);

    unsigned char iso_calc_file_name_len_l1(const ckcore::tchar *file_name, CharacterSet char_set);
    unsigned char iso_calc_file_name_len_l2(const ckcore::tchar *file_name);
    unsigned char iso_calc_file_name_len_1999(const ckcore::tchar *file_name);
    unsigned char iso_calc_dir_name_len_l1(const ckcore::tchar *file_name);
    unsigned char iso_calc_dir_name_len_l2(const ckcore::tchar *file_name);
    unsigned char iso_calc_dir_name_len_1999(const ckcore::tchar *file_name);

    void iso_make_datetime(struct tm &time, tiso_voldesc_datetime &iso_time);
    void iso_make_datetime(struct tm &time, tiso_dir_record_datetime &iso_time);
    void iso_make_datetime(ckcore::tuint16 date, ckcore::tuint16 time,
                           tiso_dir_record_datetime &iso_time);
    void iso_make_dosdatetime(tiso_dir_record_datetime &iso_time,
                              ckcore::tuint16 &date, ckcore::tuint16 &time);

    /**
     * Implements functionallity for creating parts of ISO9660 file systems.
     * For example writing certain descriptors and for generating ISO9660
     * compatible file names.
     */
    class Iso
    {
    public:
        /**
         * @brief Supported interchange levels.
         */
        enum InterchangeLevel
        {
            LEVEL_1,
            LEVEL_2,
            LEVEL_3,
            ISO9660_1999
        };

    private:
        bool relax_max_dir_level_;
        bool inc_file_ver_info_;
        InterchangeLevel inter_level_;
        CharacterSet char_set_;

        tiso_voldesc_primary voldesc_primary_;
        tiso_voldesc_setterm voldesc_setterm_;

        void init_vol_desc_primary();
        void init_vol_desc_setterm();

    public:
        Iso();
        ~Iso();

        // Change of internal state functions.
        void set_volume_label(const ckcore::tchar *label);
        void set_text_fields(const ckcore::tchar *sys_ident,const ckcore::tchar *volset_ident,
                             const ckcore::tchar *publ_ident,const ckcore::tchar *prep_ident);
        void set_file_fields(const ckcore::tchar *copy_file_ident,const ckcore::tchar *abst_file_ident,
                             const ckcore::tchar *bibl_file_ident);
        void set_interchange_level(InterchangeLevel inter_level);
        void set_char_set(CharacterSet char_set);
        void set_relax_max_dir_level(bool relax);
        void set_include_file_ver_info(bool include);

        // Write functions.
        void write_vol_desc_primary(ckcore::CanexOutStream &out_stream,struct tm &create_time,
                                    ckcore::tuint32 vol_space_size,ckcore::tuint32 pathtable_size,
                                    ckcore::tuint32 pos_pathtable_l,ckcore::tuint32 pos_pathtable_m,
                                    ckcore::tuint32 root_extent_loc,ckcore::tuint32 data_len);
        void write_vol_desc_suppl(ckcore::CanexOutStream &out_stream,struct tm &create_time,
                                  ckcore::tuint32 vol_space_size,ckcore::tuint32 pathtable_size,
                                  ckcore::tuint32 pos_pathtable_l,ckcore::tuint32 pos_pathtable_m,
                                  ckcore::tuint32 root_extent_loc,ckcore::tuint32 data_len);
        void write_vol_desc_setterm(ckcore::CanexOutStream &out_stream);

        // Helper functions.
        unsigned char write_file_name(unsigned char *buffer, const ckcore::tchar *file_name,
                                      bool is_dir);

        /**
         * Determines the length of the compatible file name generated from the
         * specified file name. A compatible file name is a file name that is
         * generated from the specified file name that is supported by the
         * current file system configuration.
         * @param file_name the origial file name.
         * @param is_dir if true, file_name is assumed to be a directory name.
         * @return the length of the compatible file name.
         */
        unsigned char calc_file_name_len(const ckcore::tchar *file_name, bool is_dir);

        /**
         * Obtains the maximum numbe of directory levels supported by the
         * current file system configuration.
         * @return maximum number of allowed directory levels.
         */
        unsigned char get_max_dir_level();

        /**
         * Checks whether the file system has a supplementary volume descriptor or not.
         * @return true if the file system has a supplementary volume descriptor.
         */
        bool has_vol_desc_suppl();

        /**
         * Checks whether the file system allows files to be fragmented
         * (multiple extents) or not.
         * @return true if the file system allows fragmentation.
         */
        bool allows_fragmentation();

        /**
         * Returns true if the file names includes the two character file
         * version information (;1).
         */
        bool includes_file_ver_info();
    };
};
