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
#include <stdio.h>
#include <ckcore/filestream.hh>
#include <ckcore/exception.hh>
#include "ckfilesystem/iso.hh"
#include "ckfilesystem/util.hh"
#include "ckfilesystem/exception.hh"
#include "ckfilesystem/eltorito.hh"

namespace ckfilesystem
{
    ElTorito::ElTorito()
    {
    }

    ElTorito::~ElTorito()
    {
        // Free the children.
        std::vector<ElToritoImage *>::iterator itImage;
        for (itImage = boot_images_.begin(); itImage != boot_images_.end(); itImage++)
            delete *itImage;

        boot_images_.clear();
    }

    bool ElTorito::read_sys_type_mbr(const ckcore::tchar *full_path,unsigned char &sys_type)
    {
        // Find the system type in the path table located in the MBR.
        ckcore::FileInStream in_file(full_path);
        if (!in_file.open())
            return false;

        teltorito_mbr mbr;

        ckcore::tint64 processed = in_file.read(&mbr,sizeof(teltorito_mbr));
        if (processed == -1)
            return false;
        if (processed != sizeof(teltorito_mbr) ||
            (mbr.signature1 != 0x55 || mbr.signature2 != 0xaa))
        {
            return false;
        }

        size_t used_partition = -1;
        for (size_t i = 0; i < MBR_PARTITION_COUNT; i++)
        {
            // Look for the first used partition.
            if (mbr.partitions[i].part_type != 0)
            {
                // Make sure that the boot image only contains one partition.
                if (used_partition != size_t(-1))
                    return false;
                else
                    used_partition = i;
            }
        }

        sys_type = mbr.partitions[used_partition].part_type;
        return true;
    }

    void ElTorito::write_boot_record(SectorOutStream &out_stream,ckcore::tuint32 boot_cat_sec_pos)
    {
        tiso_voldesc_eltorito_record br;
        memset(&br,0,sizeof(tiso_voldesc_eltorito_record));

        br.type = 0;
        memcpy(br.ident,iso_ident_cd,sizeof(br.ident));
        br.version = 1;
        memcpy(br.boot_sys_ident,iso_ident_eltorito,strlen(iso_ident_eltorito));
        br.boot_cat_ptr = boot_cat_sec_pos;

        out_stream.write(&br,sizeof(br));
    }

    void ElTorito::write_boot_catalog(SectorOutStream &out_stream)
    {
        char szManufacturer[] = { 0x49,0x4e,0x46,0x52,0x41,0x52,0x45,0x43,0x4F,0x52,0x44,0x45,0x52 };
        teltorito_valientry ve;
        memset(&ve,0,sizeof(teltorito_valientry));

        ve.header = 0x01;
        ve.platform = ELTORITO_PLATFORM_80X86;
        memcpy(ve.manufacturer,szManufacturer,13);
        ve.key_byte1 = 0x55;
        ve.key_byte2 = 0xaa;

        // Calculate check sum.
        int checksum = 0;
        unsigned char *pEntryPtr = (unsigned char *)&ve;
        for (size_t i = 0; i < sizeof(teltorito_valientry); i += 2) {
            checksum += (unsigned int)pEntryPtr[i];
            checksum += ((unsigned int)pEntryPtr[i + 1]) << 8;
        }

        ve.checksum = -checksum;

        out_stream.write(&ve,sizeof(ve));

        // write the default boot entry.
        if (boot_images_.size() < 1)
            throw ckcore::Exception2(ckT("No ElTorito images to write."));

        ElToritoImage *pDefImage = boot_images_[0];

        teltorito_defentry dbe;
        memset(&dbe,0,sizeof(teltorito_defentry));

        dbe.boot_indicator = pDefImage->bootable_ ?
            ELTORITO_BOOTINDICATOR_BOOTABLE : ELTORITO_BOOTINDICATOR_NONBOOTABLE;
        dbe.load_segment = pDefImage->load_segment_;
        dbe.sec_count = pDefImage->sec_count_;
        dbe.load_sec_addr = pDefImage->data_sec_pos_;

        switch (pDefImage->emulation_)
        {
            case ElToritoImage::EMULATION_NONE:
                dbe.emulation = ELTORITO_EMULATION_NONE;
                dbe.sys_type = 0;
                break;

            case ElToritoImage::EMULATION_FLOPPY:
                switch (ckcore::File::size2(pDefImage->full_path_.c_str()))
                {
                    case 1200 * 1024:
                        dbe.emulation = ELTORITO_EMULATION_DISKETTE12;
                        break;
                    case 1440 * 1024:
                        dbe.emulation = ELTORITO_EMULATION_DISKETTE144;
                        break;
                    case 2880 * 1024:
                        dbe.emulation = ELTORITO_EMULATION_DISKETTE288;
                        break;
                    default:
                        throw ckcore::Exception2(ckT("Invalid ElTorito floppy emulation type."));
                }

                dbe.sys_type = 0;
                break;

            case ElToritoImage::EMULATION_HARDDISK:
                dbe.emulation = ELTORITO_EMULATION_HARDDISK;

                if (!read_sys_type_mbr(pDefImage->full_path_.c_str(),dbe.sys_type))
                    throw ckcore::Exception2(ckT("Could not read MBR for producing an ElTorito boot image."));
                break;
        }

        out_stream.write(&dbe,sizeof(dbe));

        // write the rest of the boot images.
        teltorito_sec_header sh;
        teltorito_sec_entry se;

        ckcore::tuint16 usNumImages = (ckcore::tuint16)boot_images_.size();
        for (ckcore::tuint16 i = 1; i < usNumImages; i++)
        {
            ElToritoImage *pCurImage = boot_images_[i];

            // write section header.
            memset(&sh,0,sizeof(teltorito_sec_header));

            sh.header = i == (usNumImages - 1) ?
                ELTORITO_HEADER_FINAL : ELTORITO_HEADER_NORMAL;
            sh.platform = ELTORITO_PLATFORM_80X86;
            sh.num_sec_entries = 1;

            char szIdentifier[16];
            sprintf(szIdentifier,"IMAGE%u",i + 1);
            memcpy(sh.ident,szIdentifier,strlen(szIdentifier));

            out_stream.write(&sh,sizeof(sh));

            // write the section entry.
            memset(&se,0,sizeof(teltorito_sec_entry));

            se.boot_indicator = pCurImage->bootable_ ?
                ELTORITO_BOOTINDICATOR_BOOTABLE : ELTORITO_BOOTINDICATOR_NONBOOTABLE;
            se.load_segment = pCurImage->load_segment_;
            se.sec_count = pCurImage->sec_count_;
            se.load_sec_addr = pCurImage->data_sec_pos_;

            switch (pCurImage->emulation_)
            {
                case ElToritoImage::EMULATION_NONE:
                    se.emulation = ELTORITO_EMULATION_NONE;
                    se.sys_type = 0;
                    break;

                case ElToritoImage::EMULATION_FLOPPY:
                    switch (ckcore::File::size2(pCurImage->full_path_.c_str()))
                    {
                        case 1200 * 1024:
                            se.emulation = ELTORITO_EMULATION_DISKETTE12;
                            break;
                        case 1440 * 1024:
                            se.emulation = ELTORITO_EMULATION_DISKETTE144;
                            break;
                        case 2880 * 1024:
                            se.emulation = ELTORITO_EMULATION_DISKETTE288;
                            break;
                        default:
                            throw ckcore::Exception2(ckT("Invalid ElTorito floppy emulation type."));
                    }

                    se.sys_type = 0;
                    break;

                case ElToritoImage::EMULATION_HARDDISK:
                    se.emulation = ELTORITO_EMULATION_HARDDISK;

                    if (!read_sys_type_mbr(pCurImage->full_path_.c_str(),se.sys_type))
                        throw ckcore::Exception2(ckT("Could not read MBR for producing an ElTorito boot image."));
                    break;
            }

            out_stream.write(&se,sizeof(se));
        }

        if (out_stream.get_allocated() != 0)
            out_stream.pad_sector();
    }

    void ElTorito::write_boot_image(SectorOutStream &out_stream,const ckcore::tchar *full_path)
    {
        ckcore::FileInStream FileStream(full_path);
        if (!FileStream.open())
            throw FileOpenException(full_path);

        char szBuffer[ELTORITO_IO_BUFFER_SIZE];

        while (!FileStream.end())
        {
            ckcore::tint64 processed = FileStream.read(szBuffer,ELTORITO_IO_BUFFER_SIZE);
            if (processed == -1)
            {
                throw ckcore::Exception2(ckcore::string::formatstr(ckT("Unable to read the file \"%s\"."),full_path));
            }

            out_stream.write(szBuffer,(ckcore::tuint32)processed);
        }

        // Pad the sector.
        if (out_stream.get_allocated() != 0)
            out_stream.pad_sector();
    }

    void ElTorito::write_boot_images(SectorOutStream &out_stream)
    {
        std::vector<ElToritoImage *>::iterator it;
        for (it = boot_images_.begin(); it != boot_images_.end(); it++)
            write_boot_image(out_stream,(*it)->full_path_.c_str());
    }

    bool ElTorito::add_boot_image_no_emu(const ckcore::tchar *full_path,bool bootable,
                                         ckcore::tuint16 load_segment,ckcore::tuint16 sec_count)
    {
        if (boot_images_.size() >= ELTORITO_MAX_BOOTIMAGE_COUNT)
            return false;

        if (!ckcore::File::exist(full_path))
            return false;

        boot_images_.push_back(new ElToritoImage(
            full_path,bootable,ElToritoImage::EMULATION_NONE,load_segment,sec_count));
        return true;
    }

    bool ElTorito::add_boot_image_floppy(const ckcore::tchar *full_path,bool bootable)
    {
        if (boot_images_.size() >= ELTORITO_MAX_BOOTIMAGE_COUNT)
            return false;

        if (!ckcore::File::exist(full_path))
            return false;

        // Validate floppy image size.
        ckcore::tuint64 file_size = ckcore::File::size(full_path);
        if (file_size != 1200 * 1024 && file_size != 1440 * 1024 && file_size != 2880 * 1024)
            return false;

        // sec_count = 1, only load one sector for floppies.
        boot_images_.push_back(new ElToritoImage(
            full_path,bootable,ElToritoImage::EMULATION_FLOPPY,0,1));
        return true;
    }

    bool ElTorito::add_boot_image_hard_disk(const ckcore::tchar *full_path,bool bootable)
    {
        if (boot_images_.size() >= ELTORITO_MAX_BOOTIMAGE_COUNT)
            return false;

        if (!ckcore::File::exist(full_path))
            return false;

        unsigned char dummy;
        if (!read_sys_type_mbr(full_path,dummy))
            return false;

        // sec_count = 1, Only load the MBR.
        boot_images_.push_back(new ElToritoImage(
            full_path,bootable,ElToritoImage::EMULATION_HARDDISK,0,1));
        return true;
    }

    void ElTorito::calc_filesys_data(ckcore::tuint64 start_sec,
                                     ckcore::tuint64 &last_sec)
    {
        std::vector<ElToritoImage *>::iterator it;
        for (it = boot_images_.begin(); it != boot_images_.end(); it++)
        {
            if (start_sec > 0xffffffff)
                throw ckcore::Exception2(ckT("Could not calculate ElTorito data boundaries."));

            (*it)->data_sec_pos_ = (ckcore::tuint32)start_sec;

            start_sec += util::bytes_to_sec64((ckcore::tuint64)
                ckcore::File::size((*it)->full_path_.c_str()));
        }

        last_sec = start_sec;
    }

    ckcore::tuint64 ElTorito::get_boot_cat_size()
    {
        // The validator and default boot image allocates 64 bytes, the remaining
        // boot images allocates 64 bytes a piece.
        return boot_images_.size() << 6;
    }

    ckcore::tuint64 ElTorito::get_boot_data_size()
    {
        ckcore::tuint64 size = 0;
        std::vector<ElToritoImage *>::iterator it;
        for (it = boot_images_.begin(); it != boot_images_.end(); it++)
            size += util::bytes_to_sec64((ckcore::tuint64)ckcore::File::size((*it)->full_path_.c_str()));

        return size;
    }

    ckcore::tuint64 ElTorito::get_boot_image_count()
    {
        return boot_images_.size();
    }
};
