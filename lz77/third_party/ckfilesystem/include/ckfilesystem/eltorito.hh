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
#include <string>
#include <vector>
#include <ckcore/types.hh>
#include <ckcore/log.hh>
#include "ckfilesystem/sectorstream.hh"

// Maximum values of ckcore::tuint16 + 1 + the default boot image.
#define ELTORITO_MAX_BOOTIMAGE_COUNT            0xffff + 2

#define ELTORITO_IO_BUFFER_SIZE                 0x10000

#pragma pack(1) // Force byte alignment.

/*
    Boot Catalog Entries.
*/
#define ELTORITO_PLATFORM_80X86                 0
#define ELTORITO_PLATFORM_POWERPC               1
#define ELTORITO_PLATFORM_MAC                   2

typedef struct
{
    unsigned char header;
    unsigned char platform;
    ckcore::tuint16 res1;
    unsigned char manufacturer[24];
    ckcore::tuint16 checksum;
    unsigned char key_byte1;        // Must be 0x55.
    unsigned char key_byte2;        // Must be 0xaa.
} teltorito_valientry;

#define ELTORITO_BOOTINDICATOR_BOOTABLE         0x88
#define ELTORITO_BOOTINDICATOR_NONBOOTABLE      0x00

#define ELTORITO_EMULATION_NONE                 0
#define ELTORITO_EMULATION_DISKETTE12           1
#define ELTORITO_EMULATION_DISKETTE144          2
#define ELTORITO_EMULATION_DISKETTE288          3
#define ELTORITO_EMULATION_HARDDISK             4

typedef struct
{
    unsigned char boot_indicator;
    unsigned char emulation;
    ckcore::tuint16 load_segment;
    unsigned char sys_type;     // Must be a copy of byte 5 (System Type) from boot image partition table.
    unsigned char unused1;
    ckcore::tuint16 sec_count;
    ckcore::tuint32 load_sec_addr;
    unsigned char unused2[20];
} teltorito_defentry;

#define ELTORITO_HEADER_NORMAL                  0x90
#define ELTORITO_HEADER_FINAL                   0x91

typedef struct
{
    unsigned char header;
    unsigned char platform;
    ckcore::tuint16 num_sec_entries;
    unsigned char ident[28];
} teltorito_sec_header;

typedef struct
{
    unsigned char boot_indicator;
    unsigned char emulation;
    ckcore::tuint16 load_segment;
    unsigned char sys_type;     // Must be a copy of byte 5 (System Type) from boot image partition table.
    unsigned char unused1;
    ckcore::tuint16 sec_count;
    ckcore::tuint32 load_sec_addr;
    unsigned char sel_criteria;
    unsigned char unused2[19];
} teltorito_sec_entry;

/*
    Structures for reading the master boot record of a boot image.
*/
#define MBR_PARTITION_COUNT                     4

typedef struct
{
    unsigned char boot_indicator;
    unsigned char part_start_chs[3];
    unsigned char part_type;
    unsigned char part_end_chs[3];
    ckcore::tuint32 start_lba;
    ckcore::tuint32 sec_count;
} teltorito_mbr_part;

typedef struct
{
    unsigned char code_area[440];
    ckcore::tuint32 opt_disc_sig;
    ckcore::tuint16 pad;
    teltorito_mbr_part partitions[MBR_PARTITION_COUNT];
    unsigned char signature1;
    unsigned char signature2;
} teltorito_mbr;

#pragma pack()  // Switch back to normal alignment.

namespace ckfilesystem
{
    class ElToritoImage
    {
    public:
        enum Emulation
        {
            EMULATION_NONE,
            EMULATION_FLOPPY,
            EMULATION_HARDDISK
        };

        ckcore::tstring full_path_;
        bool bootable_;
        Emulation emulation_;
        ckcore::tuint16 load_segment_;
        ckcore::tuint16 sec_count_;

        // Needs to be calculated in a separate pass.
        ckcore::tuint32 data_sec_pos_;  // Sector number of first sector containing data.

        ElToritoImage(const ckcore::tchar *full_path,bool bootable,Emulation emulation,
                      ckcore::tuint16 load_segment,ckcore::tuint16 sec_count) : 
            full_path_(full_path),bootable_(bootable),emulation_(emulation),
            load_segment_(load_segment),sec_count_(sec_count),data_sec_pos_(0)
        {
        }
    };

    class ElTorito
    {
    private:
        std::vector<ElToritoImage *> boot_images_;

        bool read_sys_type_mbr(const ckcore::tchar *full_path,unsigned char &sys_type);

        void write_boot_image(SectorOutStream &out_stream,const ckcore::tchar *full_path);

    public:
        ElTorito();
        ~ElTorito();

        void write_boot_record(SectorOutStream &out_stream,ckcore::tuint32 boot_cat_sec_pos);
        void write_boot_catalog(SectorOutStream &out_stream);
        void write_boot_images(SectorOutStream &out_stream);

        bool add_boot_image_no_emu(const ckcore::tchar *full_path,bool bootable,
                                   ckcore::tuint16 load_segment,ckcore::tuint16 sec_count);
        bool add_boot_image_floppy(const ckcore::tchar *full_path,bool bootable);
        bool add_boot_image_hard_disk(const ckcore::tchar *full_path,bool bootable);

        void calc_filesys_data(ckcore::tuint64 start_sec,
                               ckcore::tuint64 &last_sec);

        ckcore::tuint64 get_boot_cat_size();
        ckcore::tuint64 get_boot_data_size();
        ckcore::tuint64 get_boot_image_count();
    };
};
