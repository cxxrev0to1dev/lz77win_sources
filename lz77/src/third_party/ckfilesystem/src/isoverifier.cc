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
#include <ckcore/exception.hh>
#include <ckcore/log.hh>
#include <ckcore/string.hh>
#include "ckfilesystem/sectorstream.hh"
#include "ckfilesystem/util.hh"
#include "ckfilesystem/iso.hh"
#include "ckfilesystem/isoverifier.hh"

namespace ckfilesystem
{
    using namespace util;

    /**
     * Constructs an IsoVolDescSet object.
     */
    IsoVolDescSet::IsoVolDescSet()
    {
        memset(&voldesc_primary_,0,sizeof(voldesc_primary_));
    }

    /**
     * Reads a volume descriptor from a stream into the object.
     * @param [in] in_stream The input stream to read from.
     * @throw Exception If an error occurred.
     */
    void IsoVolDescSet::read(SectorInStream &in_stream)
    {
        unsigned char buffer[2048];
        ckcore::log::print_line(ckT("Looking for volume descriptor set..."));

        while (!in_stream.end())
        {
            ckcore::tuint32 cur_sec =
                static_cast<ckcore::tuint32>(in_stream.get_sector());

            in_stream.read(buffer,sizeof(buffer));
            unsigned char type = buffer[0];

            switch (type)
            {
            case VOLDESCTYPE_BOOT_CATALOG:
                {
                    tiso_voldesc_bootrec tmp;
                    memcpy(&tmp,buffer,sizeof(tmp));
                    voldesc_bootrec_.push_back(tmp);
                }

                ckcore::log::print_line(ckT("  Found: Boot catalog at sector %d."),cur_sec);
                break;

            case VOLDESCTYPE_PRIM_VOL_DESC:
                tiso_voldesc_primary zero;
                memset(&zero,0,sizeof(zero));

                if (memcmp(&voldesc_primary_,&zero,sizeof(tiso_voldesc_primary)))
                {
                    ckcore::log::print_line(ckT("Warning: Found an extra primary ")
                                            ckT("volume descriptor at sector %d."),
                                            cur_sec);
                }

                memcpy(&voldesc_primary_,buffer,sizeof(voldesc_primary_));

                ckcore::log::print_line(ckT("  Found: Primary volume descriptor at sector %d."),cur_sec);
                break;

            case VOLDESCTYPE_SUPPL_VOL_DESC:
                {
                    tiso_voldesc_suppl tmp;
                    memcpy(&tmp,buffer,sizeof(tmp));
                    voldesc_suppl_.push_back(tmp);
                }

                ckcore::log::print_line(ckT("  Found: Supplementary volume descriptor at sector %d."),cur_sec);
                break;

            case VOLDESCTYPE_VOL_PARTITION_DESC:
                {
                    tiso_voldesc_part tmp;
                    memcpy(&tmp,buffer,sizeof(tmp));
                    voldesc_part_.push_back(tmp);

                    ckcore::log::print_line(ckT("  Found: Volume partition descriptor at sector %d."),cur_sec);
                }
                break;

            case VOLDESCTYPE_VOL_DESC_SET_TERM:
                {
                    tiso_voldesc_setterm tmp;
                    memcpy(&tmp,buffer,sizeof(tmp));
                    voldesc_setterm_.push_back(tmp);

                    ckcore::log::print_line(ckT("  Found: Volume set terminator descriptor at sector %d."),cur_sec);
                    ckcore::log::print_line(ckT("End of volume set found."));
                    ckcore::log::print_line(ckT(""));
                    return;
                }

            default:
                ckcore::tstringstream msg;
                msg << ckT("Invalid volume descriptor of type ") << type
                    << ckT(" at sector ") << cur_sec << ckT(".");
                throw ckcore::Exception2(msg.str());
            }
        }

        ckcore::log::print_line(ckT(""));

        throw VerificationException(ckT("Volume set did not contain a terminator."),
                                    ckT("ECMA 119: 6.7.1.5."));
    }

    /**
     * Prints and verifies a volume descriptor date and time record.
     * @param [in] voldesc_datetime The date and time record to verify.
     * @param [in] label The label to use when printing the record.
     * @throw VerificationException If the record is invalid.b
     */
    void IsoVolDescSet::verify(const tiso_voldesc_datetime &voldesc_datetime,
                               const ckcore::tchar *label) const
    {
        const char *year_str = reinterpret_cast<const char *>(&voldesc_datetime.year);
        const char *mon_str = reinterpret_cast<const char *>(&voldesc_datetime.mon);
        const char *day_str = reinterpret_cast<const char *>(&voldesc_datetime.day);
        const char *hour_str = reinterpret_cast<const char *>(&voldesc_datetime.hour);
        const char *min_str = reinterpret_cast<const char *>(&voldesc_datetime.min);
        const char *sec_str = reinterpret_cast<const char *>(&voldesc_datetime.sec);
        const char *hundreds_str = reinterpret_cast<const char *>(&voldesc_datetime.hundreds);

        ckcore::tuint32 year = (year_str[0] - '0') * 1000 + (year_str[1] - '0') * 100 +
                               (year_str[2] - '0') * 10   + (year_str[3] - '0');
        ckcore::tuint32 mon = (mon_str[0] - '0') * 10 + (mon_str[1] - '0');
        ckcore::tuint32 day = (day_str[0] - '0') * 10 + (day_str[1] - '0');

        ckcore::tuint32 hour = (hour_str[0] - '0') * 10 + (hour_str[1] - '0');
        ckcore::tuint32 min = (min_str[0] - '0') * 10 + (min_str[1] - '0');
        ckcore::tuint32 sec = (sec_str[0] - '0') * 10 + (sec_str[1] - '0');
        ckcore::tuint32 hundreds = (hundreds_str[0] - '0') * 10 + (hundreds_str[1] - '0');

        ckcore::log::print_line(ckT("%s %04d-%02d-%02d %02d:%02d:%02d(:%02d) in zone %d"),
                                label,year,mon,day,hour,min,sec,hundreds,voldesc_datetime.zone);

        if (year == 0 && mon == 0 && day == 0 && hour == 0 &&
            min == 0 && sec == 0 && hundreds == 0 && voldesc_datetime.zone == 0)
        {
            return;
        }

        // Verify the year.
        if (year < 1 || year > 9999)
        {
            throw VerificationException(ckT("The year must be a value between 1 and 9999."),
                                        ckT("ECMA 119: 8.4.26.1."));
        }

        // Verify the month.
        if (mon < 1 || mon > 12)
        {
            throw VerificationException(ckT("The month must be a value between 1 and 12."),
                                        ckT("ECMA 119: 8.4.26.1."));
        }

        // Verify the day.
        if (day < 1 || day > 31)
        {
            throw VerificationException(ckT("The day must be a value between 1 and 31."),
                                        ckT("ECMA 119: 8.4.26.1."));
        }

        // Verify the hours.
        if (hour < 0 || hour > 23)
        {
            throw VerificationException(ckT("The hour must be a value between 0 and 23."),
                                        ckT("ECMA 119: 8.4.26.1."));
        }

        // Verify the minutes.
        if (min < 0 || min > 59)
        {
            throw VerificationException(ckT("The minute must be a value between 0 and 59."),
                                        ckT("ECMA 119: 8.4.26.1."));
        }

        // Verify the seconds.
        if (sec < 0 || sec > 59)
        {
            throw VerificationException(ckT("The second must be a value between 0 and 59."),
                                        ckT("ECMA 119: 8.4.26.1."));
        }
    }

    /**
     * Prints and verifies the specified primary descriptor.
     * @param [in] voldesc_primary The primary volume descriptor to verify.
     * @throw VerificationsException If an error occurred.
     */
    void IsoVolDescSet::verify(tiso_voldesc_primary &voldesc_primary)
    {
        char str_buffer[1024];
        size_t len = 0;

        ckcore::log::print_line(ckT("Primary volume descriptor:"));

        // Volume descriptor type.
        ckcore::log::print_line(ckT("  Volume descriptor type: %d"),
                                static_cast<ckcore::tuint32>(voldesc_primary.type));

        // Standard identifier.
        memcpy(str_buffer,voldesc_primary.ident,sizeof(voldesc_primary.ident));
        str_buffer[sizeof(voldesc_primary.ident)] = '\0';
        ckcore::log::print_line(ckT("  Standard identifier: \"%s\""),
                                ckcore::string::ansi_to_auto<1024>(str_buffer).c_str());

        if (memcmp(voldesc_primary.ident,"CD001",sizeof(voldesc_primary.ident)))
        {
            throw VerificationException(ckT("Invalid primary volume descriptor indentifier."),
                                        ckT("ECMA 110: 8.4.2."));
        }

        // Volume descriptor version.
        ckcore::log::print_line(ckT("  Volume descriptor version: %d"),
                                static_cast<ckcore::tuint32>(voldesc_primary.version));

        if (voldesc_primary.version != 1)
        {
            ckcore::log::print_line(ckT("=> Warning: Descriptor version is not 1."));
            ckcore::log::print_line(ckT("=>          See ECMA 119: 8.4.3."));
        }

        // System identifier.
        IsoVerifier::read_a_chars(voldesc_primary.sys_ident,
                                  sizeof(voldesc_primary.sys_ident),str_buffer);
        ckcore::log::print_line(ckT("  System identifier: \"%s\""),
            ckcore::string::ansi_to_auto<1024>(str_buffer).c_str());
        IsoVerifier::verify_a_chars(voldesc_primary.sys_ident,len);

        // Volume identifier.
        IsoVerifier::read_d_chars(voldesc_primary.vol_ident,
                                  sizeof(voldesc_primary.vol_ident),str_buffer);
        ckcore::log::print_line(ckT("  Volume identifier: \"%s\""),
            ckcore::string::ansi_to_auto<1024>(str_buffer).c_str());
        IsoVerifier::verify_d_chars(voldesc_primary.vol_ident,len);

        // Volume space size.
        ckcore::log::print_line(ckT("  Volume space size: %d blocks"),
                                read733(voldesc_primary.vol_space_size));

        ckcore::log::print_line(ckT("  Volume set size: %d"),
                                read723(voldesc_primary.volset_size));

        ckcore::log::print_line(ckT("  Volume sequence number: %d"),
                                read723(voldesc_primary.volseq_num));

        ckcore::log::print_line(ckT("  Logical block size: %d bytes"),
                                read723(voldesc_primary.logical_block_size));

        ckcore::log::print_line(ckT("  Path table size: %d bytes"),
                                read733(voldesc_primary.path_table_size));

        ckcore::log::print_line(ckT("  Location of occurrence of type L path table: sector %d"),
                                read731(voldesc_primary.path_table_type_l));

        ckcore::log::print_line(ckT("  Location of optional occurrence of type L path table: sector %d"),
                                read731(voldesc_primary.opt_path_table_type_l));

        ckcore::log::print_line(ckT("  Location of occurrence of type M path table: sector %d"),
                                read721(voldesc_primary.path_table_type_m));

        ckcore::log::print_line(ckT("  Location of optional occurrence of type M path table: sector %d"),
                                read731(voldesc_primary.opt_path_table_type_m));

        // Root directory record.
        ckcore::log::print_line(ckT("  Directory record for root directory:"));

        ckcore::log::print_line(ckT("    Length of directory record: %d bytes"),
                                static_cast<ckcore::tuint32>(voldesc_primary.root_dir_record.dir_record_len));
        ckcore::log::print_line(ckT("    Extended attribute record length: %d bytes"),
                                static_cast<ckcore::tuint32>(voldesc_primary.root_dir_record.ext_attr_record_len));
        ckcore::log::print_line(ckT("    Location of extent: sector %d"),
            read733(voldesc_primary.root_dir_record.extent_loc));
        ckcore::log::print_line(ckT("    Data length: %d bytes"),
            read733(voldesc_primary.root_dir_record.data_len));

        ckcore::log::print_line(ckT("    Recording date and time: %04d-%02d-%02d %02d:%02d:%02d in zone %d"),
            voldesc_primary.root_dir_record.rec_timestamp.year + 1900,
            voldesc_primary.root_dir_record.rec_timestamp.mon,
            voldesc_primary.root_dir_record.rec_timestamp.day,
            voldesc_primary.root_dir_record.rec_timestamp.hour,
            voldesc_primary.root_dir_record.rec_timestamp.min,
            voldesc_primary.root_dir_record.rec_timestamp.sec,
            voldesc_primary.root_dir_record.rec_timestamp.zone);

        if (voldesc_primary.root_dir_record.rec_timestamp.mon < 1 ||
            voldesc_primary.root_dir_record.rec_timestamp.mon > 12)
        {
            throw VerificationException(ckT("The month must be a value between 1 and 12."),
                                        ckT("ECMA 119: 9.1.5."));
        }

        if (voldesc_primary.root_dir_record.rec_timestamp.day < 1 ||
            voldesc_primary.root_dir_record.rec_timestamp.day > 31)
        {
            throw VerificationException(ckT("The day must be a value between 1 and 31."),
                                        ckT("ECMA 119: 9.1.5."));
        }

        if (voldesc_primary.root_dir_record.rec_timestamp.hour < 0 ||
            voldesc_primary.root_dir_record.rec_timestamp.hour > 23)
        {
            throw VerificationException(ckT("The hour must be a value between 0 and 23."),
                                        ckT("ECMA 119: 9.1.5."));
        }

        if (voldesc_primary.root_dir_record.rec_timestamp.min < 0 ||
            voldesc_primary.root_dir_record.rec_timestamp.min > 59)
        {
            throw VerificationException(ckT("The minute must be a value between 0 and 59."),
                                        ckT("ECMA 119: 9.1.5."));
        }

        if (voldesc_primary.root_dir_record.rec_timestamp.sec < 0 ||
            voldesc_primary.root_dir_record.rec_timestamp.sec > 59)
        {
            throw VerificationException(ckT("The second must be a value between 0 and 59."),
                                        ckT("ECMA 119: 9.1.5."));
        }

        ckcore::tstring flags;
        if (voldesc_primary.root_dir_record.file_flags & DIRRECORD_FILEFLAG_HIDDEN)
            flags += ckT("existence,");
        if (voldesc_primary.root_dir_record.file_flags & DIRRECORD_FILEFLAG_DIRECTORY)
            flags += ckT("directory,");
        if (voldesc_primary.root_dir_record.file_flags & DIRRECORD_FILEFLAG_ASSOCIATEDFILE)
            flags += ckT("associated file,");
        if (voldesc_primary.root_dir_record.file_flags & DIRRECORD_FILEFLAG_RECORD)
            flags += ckT("record,");
        if (voldesc_primary.root_dir_record.file_flags & DIRRECORD_FILEFLAG_PROTECTION)
            flags += ckT("protection,");
        if (voldesc_primary.root_dir_record.file_flags & DIRRECORD_FILEFLAG_MULTIEXTENT)
            flags += ckT("multi-extent,");

        if (flags.size() > 0)
            flags.resize(flags.size() - 1);

        ckcore::log::print_line(ckT("    File flags: %d (%s)"),
                                static_cast<ckcore::tuint32>(voldesc_primary.root_dir_record.file_flags),
                                flags.c_str());

        if (voldesc_primary.root_dir_record.file_flags & DIRRECORD_FILEFLAG_DIRECTORY)
        {
            if (voldesc_primary.root_dir_record.file_flags & DIRRECORD_FILEFLAG_ASSOCIATEDFILE)
            {
                throw VerificationException(ckT("Directories cannot have the <associated file> flag set."),
                                            ckT("ECMA 119: 9.1.6."));
            }

            if (voldesc_primary.root_dir_record.file_flags & DIRRECORD_FILEFLAG_RECORD)
            {
                throw VerificationException(ckT("Directories cannot have the <record> flag set."),
                                            ckT("ECMA 119: 9.1.6."));
            }

            if (voldesc_primary.root_dir_record.file_flags & DIRRECORD_FILEFLAG_MULTIEXTENT)
            {
                throw VerificationException(ckT("Directories cannot have the <multi-extent> flag set."),
                                            ckT("ECMA 119: 9.1.6."));
            }
        }

        ckcore::log::print_line(ckT("    File unit size: %d"),
                                static_cast<ckcore::tuint32>(voldesc_primary.root_dir_record.file_unit_size));

        ckcore::log::print_line(ckT("    Interleave gap size: %d"),
                                static_cast<ckcore::tuint32>(voldesc_primary.root_dir_record.interleave_gap_size));

        ckcore::log::print_line(ckT("    Volume sequence number: %d"),
            read723(voldesc_primary.root_dir_record.volseq_num));

        ckcore::log::print_line(ckT("    File identifier length: %d"),
                                static_cast<ckcore::tuint32>(voldesc_primary.root_dir_record.file_ident_len));

        if (voldesc_primary.root_dir_record.file_ident_len != 1 ||
            voldesc_primary.root_dir_record.file_ident[0] != 0)
        {
            ckcore::tstringstream msg;
            msg << ckT("Invalid file name identifier ")
                << voldesc_primary.root_dir_record.file_ident[0]
                << ckT(" of length ") << voldesc_primary.root_dir_record.file_ident_len
                << ckT(" in root directory record.");
            throw VerificationException(msg.str(),ckT("ECMA 119: 6.8.2.2."));
        }

        ckcore::log::print_line(ckT("    File identifier: 0"));

        /*memcpy(str_buffer,voldesc_primary.root_dir_record.file_ident,
               voldesc_primary.root_dir_record.file_ident_len);
        str_buffer[voldesc_primary.root_dir_record.file_ident_len] = '\0';
        ckcore::log::print_line(ckT("    File identifier: \"%s\""),
            ckcore::string::ansi_to_auto<1024>(str_buffer).c_str());
        IsoVerifier::verify_d_chars(voldesc_primary.root_dir_record.file_ident,
                                    voldesc_primary.root_dir_record.file_ident_len,
                                    !(voldesc_primary.root_dir_record.file_flags & DIRRECORD_FILEFLAG_DIRECTORY));*/

        // Volume set identifier.
        IsoVerifier::read_d_chars(voldesc_primary.volset_ident,
                                  sizeof(voldesc_primary.volset_ident),str_buffer);
        ckcore::log::print_line(ckT("  Volume set identifier: \"%s\""),
            ckcore::string::ansi_to_auto<1024>(str_buffer).c_str());
        IsoVerifier::verify_d_chars(voldesc_primary.volset_ident,len);

        // Publisher identifier.
        IsoVerifier::read_a_chars(voldesc_primary.publ_ident,
                                  sizeof(voldesc_primary.publ_ident),str_buffer);
        ckcore::log::print_line(ckT("  Publisher identifier: \"%s\""),
            ckcore::string::ansi_to_auto<1024>(str_buffer).c_str());
        IsoVerifier::verify_a_chars(voldesc_primary.publ_ident,len);

        // Data preparer identifier.
        IsoVerifier::read_a_chars(voldesc_primary.prep_ident,
                                  sizeof(voldesc_primary.prep_ident),str_buffer);
        ckcore::log::print_line(ckT("  Data preparer identifier: \"%s\""),
            ckcore::string::ansi_to_auto<1024>(str_buffer).c_str());
        IsoVerifier::verify_a_chars(voldesc_primary.prep_ident,len);

        // Application identifier.
        IsoVerifier::read_a_chars(voldesc_primary.app_ident,
                                  sizeof(voldesc_primary.app_ident),str_buffer);
        ckcore::log::print_line(ckT("  Application identifier: \"%s\""),
            ckcore::string::ansi_to_auto<1024>(str_buffer).c_str());
        IsoVerifier::verify_a_chars(voldesc_primary.app_ident,len);

        // Copyright file identifier.
        IsoVerifier::read_d_chars(voldesc_primary.copy_file_ident,
                                  sizeof(voldesc_primary.copy_file_ident),str_buffer);
        ckcore::log::print_line(ckT("  Copyright file identifier: \"%s\""),
            ckcore::string::ansi_to_auto<1024>(str_buffer).c_str());
        IsoVerifier::verify_d_chars(voldesc_primary.copy_file_ident,len,true);

        // Abstract file identifier.
        IsoVerifier::read_d_chars(voldesc_primary.abst_file_ident,
                                  sizeof(voldesc_primary.abst_file_ident),str_buffer);
        ckcore::log::print_line(ckT("  Abstract file identifier: \"%s\""),
            ckcore::string::ansi_to_auto<1024>(str_buffer).c_str());
        IsoVerifier::verify_d_chars(voldesc_primary.abst_file_ident,len,true);

        // Bibliographic file identifier.
        IsoVerifier::read_d_chars(voldesc_primary.bibl_file_ident,
                                  sizeof(voldesc_primary.bibl_file_ident),str_buffer);
        ckcore::log::print_line(ckT("  Bibliographic file identifier: \"%s\""),
            ckcore::string::ansi_to_auto<1024>(str_buffer).c_str());
        IsoVerifier::verify_d_chars(voldesc_primary.bibl_file_ident,len,true);

        // Volume date and times.
        verify(voldesc_primary.create_time,ckT("  Volume creation date and time:"));
        verify(voldesc_primary.modify_time,ckT("  Volume modification date and time:"));
        verify(voldesc_primary.expr_time,ckT("  Volume expiration date and time:"));
        verify(voldesc_primary.effect_time,ckT("  Volume effective date and time:"));

        // File structure version.
        ckcore::log::print_line(ckT("  File structure version: %d"),
                                static_cast<ckcore::tuint32>(voldesc_primary.file_struct_ver));

        unsigned char app_data_zero[512];
        memset(app_data_zero,0,sizeof(app_data_zero));
        if (!memcmp(voldesc_primary.app_data,app_data_zero,sizeof(voldesc_primary.app_data)))
            ckcore::log::print_line(ckT("  Application use: <no>"));
        else
            ckcore::log::print_line(ckT("  Application use: <yes>"));

        unsigned char unused5_zero[653];
        memset(unused5_zero,0,sizeof(unused5_zero));
        if (memcmp(voldesc_primary.unused5,unused5_zero,sizeof(voldesc_primary.unused5)))
        {
            ckcore::log::print_line(ckT("=> Warning: Reserved data in primary volume descriptor is used."));
            ckcore::log::print_line(ckT("=>          See ECMA 119: 8.4.33."));
        }

        ckcore::log::print_line(ckT(""));
    }

    /**
     * Prints and verifies the specified supplementary descriptor.
     * @param [in] voldesc_suppl The supplementary volume descriptor to verify.
     * @param [in] index The index of the specified descriptor.
     * @throw VerificationsException If an error occurred.
     */
    void IsoVolDescSet::verify(const tiso_voldesc_suppl &voldesc_suppl,int index) const
    {
        wchar_t str_buffer[1024];
        size_t len = 0;

        ckcore::log::print_line(ckT("Supplementary volume descriptor %d:"),index);

        // Volume descriptor type.
        ckcore::log::print_line(ckT("  Volume descriptor type: %d"),
                                static_cast<ckcore::tuint32>(voldesc_suppl.type));

        // Standard identifier.
        char ident[1024];
        memcpy(ident,voldesc_suppl.ident,sizeof(voldesc_suppl.ident));
        str_buffer[sizeof(voldesc_suppl.ident)] = '\0';
        ckcore::log::print_line(ckT("  Standard identifier: \"%s\""),
                                ckcore::string::ansi_to_auto<1024>(ident).c_str());

        if (memcmp(voldesc_suppl.ident,"CD001",sizeof(voldesc_suppl.ident)))
        {
            throw VerificationException(ckT("Invalid primary volume descriptor indentifier."),
                                        ckT("ECMA 110: 8.4.2."));
        }

        // Volume descriptor version.
        ckcore::log::print_line(ckT("  Volume descriptor version: %d"),
                                static_cast<ckcore::tuint32>(voldesc_suppl.version));

        if (voldesc_suppl.version != 1)
        {
            ckcore::log::print_line(ckT("=> Warning: Descriptor version is not 1."));
            ckcore::log::print_line(ckT("=>          See ECMA 119: 8.4.3."));
        }

        // Volume flags.
        ckcore::log::print_line(ckT("  Volume flags: %d"),
                                static_cast<ckcore::tuint32>(voldesc_suppl.vol_flags));

        // System identifier.
        IsoVerifier::read_j_chars(voldesc_suppl.sys_ident,
                                  sizeof(voldesc_suppl.sys_ident),str_buffer);
        ckcore::log::print_line(ckT("  System identifier: \"%s\""),
                                ckcore::string::utf16_to_auto<1024>(str_buffer).c_str());
        IsoVerifier::verify_j_chars(voldesc_suppl.sys_ident,len);

        // Volume identifier.
        IsoVerifier::read_j_chars(voldesc_suppl.vol_ident,
                                  sizeof(voldesc_suppl.vol_ident),str_buffer);
        ckcore::log::print_line(ckT("  Volume identifier: \"%s\""),
                                ckcore::string::utf16_to_auto<1024>(str_buffer).c_str());
        IsoVerifier::verify_j_chars(voldesc_suppl.vol_ident,len);

        // Volume space size.
        ckcore::log::print_line(ckT("  Volume space size: %d blocks"),
                                read733(voldesc_suppl.vol_space_size));
        // Escape sequences.
        ckcore::log::print_line(ckT("  Escape sequences: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x")
                                ckT("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x")
                                ckT("%02x%02x%02x%02x%02x%02x%02x"),
            voldesc_suppl.esc_sec[ 0],voldesc_suppl.esc_sec[ 1],voldesc_suppl.esc_sec[ 2],
            voldesc_suppl.esc_sec[ 3],voldesc_suppl.esc_sec[ 4],voldesc_suppl.esc_sec[ 5],
            voldesc_suppl.esc_sec[ 6],voldesc_suppl.esc_sec[ 7],voldesc_suppl.esc_sec[ 8],
            voldesc_suppl.esc_sec[ 9],voldesc_suppl.esc_sec[10],voldesc_suppl.esc_sec[11],
            voldesc_suppl.esc_sec[12],voldesc_suppl.esc_sec[13],voldesc_suppl.esc_sec[14],
            voldesc_suppl.esc_sec[15],voldesc_suppl.esc_sec[16],voldesc_suppl.esc_sec[17],
            voldesc_suppl.esc_sec[18],voldesc_suppl.esc_sec[19],voldesc_suppl.esc_sec[20],
            voldesc_suppl.esc_sec[21],voldesc_suppl.esc_sec[22],voldesc_suppl.esc_sec[23],
            voldesc_suppl.esc_sec[24],voldesc_suppl.esc_sec[25],voldesc_suppl.esc_sec[26],
            voldesc_suppl.esc_sec[27],voldesc_suppl.esc_sec[28],voldesc_suppl.esc_sec[29],
            voldesc_suppl.esc_sec[30],voldesc_suppl.esc_sec[31]);

        if (!(voldesc_suppl.esc_sec[0] == 0x25 && voldesc_suppl.esc_sec[1] == 0x2f &&
             (voldesc_suppl.esc_sec[2] == 0x40 || voldesc_suppl.esc_sec[2] == 0x43 ||
              voldesc_suppl.esc_sec[2] == 0x45)))
        {
            throw VerificationException(ckT("Unknown escape sequence in supplementary volume ")
                                        ckT("descriptor, string entries have possibly been wrongly interpreted."),
                                        ckT("Joliet Specification: SVD Escape Sequences Field"));
        }

        ckcore::log::print_line(ckT("  Volume set size: %d"),
                                read723(voldesc_suppl.volset_size));

        ckcore::log::print_line(ckT("  Volume sequence number: %d"),
                                read723(voldesc_suppl.volseq_num));

        ckcore::log::print_line(ckT("  Logical block size: %d bytes"),
                                read723(voldesc_suppl.logical_block_size));

        ckcore::log::print_line(ckT("  Path table size: %d bytes"),
                                read733(voldesc_suppl.path_table_size));

        ckcore::log::print_line(ckT("  Location of occurrence of type L path table: sector %d"),
                                read731(voldesc_suppl.path_table_type_l));

        ckcore::log::print_line(ckT("  Location of optional occurrence of type L path table: sector %d"),
                                read731(voldesc_suppl.opt_path_table_type_l));

        ckcore::log::print_line(ckT("  Location of occurrence of type M path table: sector %d"),
                                read721(voldesc_suppl.path_table_type_m));

        ckcore::log::print_line(ckT("  Location of optional occurrence of type M path table: sector %d"),
                                read731(voldesc_suppl.opt_path_table_type_m));

        // Root directory record.
        ckcore::log::print_line(ckT("  Directory record for root directory:"));

        ckcore::log::print_line(ckT("    Length of directory record: %d bytes"),
                                static_cast<ckcore::tuint32>(voldesc_suppl.root_dir_record.dir_record_len));
        ckcore::log::print_line(ckT("    Extended attribute record length: %d bytes"),
                                static_cast<ckcore::tuint32>(voldesc_suppl.root_dir_record.ext_attr_record_len));
        ckcore::log::print_line(ckT("    Location of extent: sector %d"),
            read733(voldesc_suppl.root_dir_record.extent_loc));
        ckcore::log::print_line(ckT("    Data length: %d bytes"),
            read733(voldesc_suppl.root_dir_record.data_len));

        ckcore::log::print_line(ckT("    Recording date and time: %04d-%02d-%02d %02d:%02d:%02d in zone %d"),
            voldesc_suppl.root_dir_record.rec_timestamp.year + 1900,
            voldesc_suppl.root_dir_record.rec_timestamp.mon,
            voldesc_suppl.root_dir_record.rec_timestamp.day,
            voldesc_suppl.root_dir_record.rec_timestamp.hour,
            voldesc_suppl.root_dir_record.rec_timestamp.min,
            voldesc_suppl.root_dir_record.rec_timestamp.sec,
            voldesc_suppl.root_dir_record.rec_timestamp.zone);

        if (voldesc_suppl.root_dir_record.rec_timestamp.mon < 1 ||
            voldesc_suppl.root_dir_record.rec_timestamp.mon > 12)
        {
            throw VerificationException(ckT("The month must be a value between 1 and 12."),
                                        ckT("ECMA 119: 9.1.5."));
        }

        if (voldesc_suppl.root_dir_record.rec_timestamp.day < 1 ||
            voldesc_suppl.root_dir_record.rec_timestamp.day > 31)
        {
            throw VerificationException(ckT("The day must be a value between 1 and 31."),
                                        ckT("ECMA 119: 9.1.5."));
        }

        if (voldesc_suppl.root_dir_record.rec_timestamp.hour < 0 ||
            voldesc_suppl.root_dir_record.rec_timestamp.hour > 23)
        {
            throw VerificationException(ckT("The hour must be a value between 0 and 23."),
                                        ckT("ECMA 119: 9.1.5."));
        }

        if (voldesc_suppl.root_dir_record.rec_timestamp.min < 0 ||
            voldesc_suppl.root_dir_record.rec_timestamp.min > 59)
        {
            throw VerificationException(ckT("The minute must be a value between 0 and 59."),
                                        ckT("ECMA 119: 9.1.5."));
        }

        if (voldesc_suppl.root_dir_record.rec_timestamp.sec < 0 ||
            voldesc_suppl.root_dir_record.rec_timestamp.sec > 59)
        {
            throw VerificationException(ckT("The second must be a value between 0 and 59."),
                                        ckT("ECMA 119: 9.1.5."));
        }

        ckcore::tstring flags;
        if (voldesc_suppl.root_dir_record.file_flags & DIRRECORD_FILEFLAG_HIDDEN)
            flags += ckT("existence,");
        if (voldesc_suppl.root_dir_record.file_flags & DIRRECORD_FILEFLAG_DIRECTORY)
            flags += ckT("directory,");
        if (voldesc_suppl.root_dir_record.file_flags & DIRRECORD_FILEFLAG_ASSOCIATEDFILE)
            flags += ckT("associated file,");
        if (voldesc_suppl.root_dir_record.file_flags & DIRRECORD_FILEFLAG_RECORD)
            flags += ckT("record,");
        if (voldesc_suppl.root_dir_record.file_flags & DIRRECORD_FILEFLAG_PROTECTION)
            flags += ckT("protection,");
        if (voldesc_suppl.root_dir_record.file_flags & DIRRECORD_FILEFLAG_MULTIEXTENT)
            flags += ckT("multi-extent,");

        if (flags.size() > 0)
            flags.resize(flags.size() - 1);

        ckcore::log::print_line(ckT("    File flags: %d (%s)"),
                                static_cast<ckcore::tuint32>(voldesc_suppl.root_dir_record.file_flags),
                                flags.c_str());

        if (voldesc_suppl.root_dir_record.file_flags & DIRRECORD_FILEFLAG_DIRECTORY)
        {
            if (voldesc_suppl.root_dir_record.file_flags & DIRRECORD_FILEFLAG_ASSOCIATEDFILE)
            {
                throw VerificationException(ckT("Directories cannot have the <associated file> flag set."),
                                            ckT("ECMA 119: 9.1.6."));
            }

            if (voldesc_suppl.root_dir_record.file_flags & DIRRECORD_FILEFLAG_RECORD)
            {
                throw VerificationException(ckT("Directories cannot have the <record> flag set."),
                                            ckT("ECMA 119: 9.1.6."));
            }

            if (voldesc_suppl.root_dir_record.file_flags & DIRRECORD_FILEFLAG_MULTIEXTENT)
            {
                throw VerificationException(ckT("Directories cannot have the <multi-extent> flag set."),
                                            ckT("ECMA 119: 9.1.6."));
            }
        }

        ckcore::log::print_line(ckT("    File unit size: %d"),
                                static_cast<ckcore::tuint32>(voldesc_suppl.root_dir_record.file_unit_size));

        ckcore::log::print_line(ckT("    Interleave gap size: %d"),
                                static_cast<ckcore::tuint32>(voldesc_suppl.root_dir_record.interleave_gap_size));

        ckcore::log::print_line(ckT("    Volume sequence number: %d"),
            read723(voldesc_suppl.root_dir_record.volseq_num));

        ckcore::log::print_line(ckT("    File identifier length: %d"),
                                static_cast<ckcore::tuint32>(voldesc_suppl.root_dir_record.file_ident_len));

        if (voldesc_suppl.root_dir_record.file_ident_len != 1 ||
            voldesc_suppl.root_dir_record.file_ident[0] != 0)
        {
            ckcore::tstringstream msg;
            msg << ckT("Invalid file name identifier ")
                << voldesc_suppl.root_dir_record.file_ident[0]
                << ckT(" of length ") << voldesc_suppl.root_dir_record.file_ident_len
                << ckT(" in root directory record.");
            throw VerificationException(msg.str(),ckT("ECMA 119: 6.8.2.2."));
        }

        ckcore::log::print_line(ckT("    File identifier: 0"));

        // Volume set identifier.
        IsoVerifier::read_j_chars(voldesc_suppl.volset_ident,
                                  sizeof(voldesc_suppl.volset_ident),str_buffer);
        ckcore::log::print_line(ckT("  Volume set identifier: \"%s\""),
            ckcore::string::utf16_to_auto<1024>(str_buffer).c_str());
        IsoVerifier::verify_j_chars(voldesc_suppl.volset_ident,len);

        // Publisher identifier.
        IsoVerifier::read_j_chars(voldesc_suppl.publ_ident,
                                  sizeof(voldesc_suppl.publ_ident),str_buffer);
        ckcore::log::print_line(ckT("  Publisher identifier: \"%s\""),
            ckcore::string::utf16_to_auto<1024>(str_buffer).c_str());
        IsoVerifier::verify_j_chars(voldesc_suppl.publ_ident,len);

        // Data preparer identifier.
        IsoVerifier::read_j_chars(voldesc_suppl.prep_ident,
                                  sizeof(voldesc_suppl.prep_ident),str_buffer);
        ckcore::log::print_line(ckT("  Data preparer identifier: \"%s\""),
            ckcore::string::utf16_to_auto<1024>(str_buffer).c_str());
        IsoVerifier::verify_j_chars(voldesc_suppl.prep_ident,len);

        // Application identifier.
        IsoVerifier::read_j_chars(voldesc_suppl.app_ident,
                                  sizeof(voldesc_suppl.app_ident),str_buffer);
        ckcore::log::print_line(ckT("  Application identifier: \"%s\""),
            ckcore::string::utf16_to_auto<1024>(str_buffer).c_str());
        IsoVerifier::verify_j_chars(voldesc_suppl.app_ident,len);

        // Copyright file identifier.
        IsoVerifier::read_j_chars(voldesc_suppl.copy_file_ident,
                                  sizeof(voldesc_suppl.copy_file_ident),str_buffer);
        ckcore::log::print_line(ckT("  Copyright file identifier: \"%s\""),
            ckcore::string::utf16_to_auto<1024>(str_buffer).c_str());
        IsoVerifier::verify_j_chars(voldesc_suppl.copy_file_ident,len);

        // Abstract file identifier.
        IsoVerifier::read_j_chars(voldesc_suppl.abst_file_ident,
                                  sizeof(voldesc_suppl.abst_file_ident),str_buffer);
        ckcore::log::print_line(ckT("  Abstract file identifier: \"%s\""),
            ckcore::string::utf16_to_auto<1024>(str_buffer).c_str());
        IsoVerifier::verify_j_chars(voldesc_suppl.abst_file_ident,len);

        // Bibliographic file identifier.
        IsoVerifier::read_j_chars(voldesc_suppl.bibl_file_ident,
                                  sizeof(voldesc_suppl.bibl_file_ident),str_buffer);
        ckcore::log::print_line(ckT("  Bibliographic file identifier: \"%s\""),
            ckcore::string::utf16_to_auto<1024>(str_buffer).c_str());
        IsoVerifier::verify_j_chars(voldesc_suppl.bibl_file_ident,len);

        // Volume date and times.
        verify(voldesc_suppl.create_time,ckT("  Volume creation date and time:"));
        verify(voldesc_suppl.modify_time,ckT("  Volume modification date and time:"));
        verify(voldesc_suppl.expr_time,ckT("  Volume expiration date and time:"));
        verify(voldesc_suppl.effect_time,ckT("  Volume effective date and time:"));

        // File structure version.
        ckcore::log::print_line(ckT("  File structure version: %d"),
                                static_cast<ckcore::tuint32>(voldesc_suppl.file_struct_ver));

        unsigned char app_data_zero[512];
        memset(app_data_zero,0,sizeof(app_data_zero));
        if (!memcmp(voldesc_suppl.app_data,app_data_zero,sizeof(voldesc_suppl.app_data)))
            ckcore::log::print_line(ckT("  Application use: <no>"));
        else
            ckcore::log::print_line(ckT("  Application use: <yes>"));

        unsigned char unused3_zero[653];
        memset(unused3_zero,0,sizeof(unused3_zero));
        if (memcmp(voldesc_suppl.unused3,unused3_zero,sizeof(voldesc_suppl.unused3)))
        {
            ckcore::log::print_line(ckT("=> Warning: Reserved data in primary volume descriptor is used."));
            ckcore::log::print_line(ckT("=>          See ECMA 119: 8.4.33."));
        }

        ckcore::log::print_line(ckT(""));
    }

    /**
     * Prints and verifies the contents of the volume descriptor set.
     * @throw Exception If an error occurred.
     */
    void IsoVolDescSet::verify()
    {
        // Make sure that there is a primary volume descriptor.
        tiso_voldesc_primary zero;
        memset(&zero,0,sizeof(zero));

        if (!memcmp(&voldesc_primary_,&zero,sizeof(tiso_voldesc_primary)))
        {
            throw VerificationException(ckT("Volume set lacks primary volume descriptor. "),
                                        ckT("ECMA 119: 6.7.1.1."));
        }

        // Make sure that there is at least one set terminator.
        if (voldesc_setterm_.size() == 0)
        {
            throw VerificationException(ckT("Volume set did not contain a terminator."),
                                        ckT("ECMA 119: 6.7.1.5."));
        }

        verify(voldesc_primary_);

        // Verify all supplementary volume descriptors.
        std::vector<tiso_voldesc_suppl>::const_iterator it;
        int i = 0;
        for (it = voldesc_suppl_.begin(); it != voldesc_suppl_.end(); it++,i++)
            verify(*it,i);
    }

    /**
     * Constructs an IsoVerifier object.
     */
    IsoVerifier::IsoVerifier()// : voldesc_primary_(NULL)
    {
    }

    /**
     * Destructs the IsoVerifier object.
     */
    IsoVerifier::~IsoVerifier()
    {
    }

    void IsoVerifier::read_vol_desc(SectorInStream &in_stream)
    {
        unsigned char buffer[2048];
        in_stream.read(buffer,sizeof(buffer));

        //tiso_voldesc_primary
        unsigned char type = buffer[0];

        switch (type)
        {
        case VOLDESCTYPE_BOOT_CATALOG:
            //read_boot_cat(in_stream);
            break;

        case VOLDESCTYPE_PRIM_VOL_DESC:
            //if (voldesc_primary_ != NULL)
            {
                ckcore::tstringstream msg;
                msg << ckT("Encountered a second primary volume descriptor at sector ")
                    << (in_stream.get_sector() - 1) << ckT(".");
                throw ckcore::Exception2(msg.str());
            }
            break;

        case VOLDESCTYPE_SUPPL_VOL_DESC:
            break;

        case VOLDESCTYPE_VOL_PARTITION_DESC:
            break;

        case VOLDESCTYPE_VOL_DESC_SET_TERM:
            break;

        default:
            ckcore::tstringstream msg;
            msg << ckT("Invalid volume descriptor of type ") << type
                << ckT(" at sector ") << (in_stream.get_sector() - 1) << ckT(".");
            throw ckcore::Exception2(msg.str());
        }
    }

    /**
     * Resets the internal state of the verifier, making it ready for a new
     * verification analysis.
     */
    void IsoVerifier::reset()
    {
    }

    /**
     * Verifies the file system provided by the specified input stream.
     * @throw Exception on any error.
     */
    void IsoVerifier::verify(SectorInStream &in_stream)
    {
        reset();

        //ckcore::log::print_line(ckT("Analyzing: "),in_stream.);

        // Skip the first 16 sectors.
        in_stream.seek(16 * 2048,ckcore::InStream::ckSTREAM_BEGIN);

        //read_vol_desc(in_stream);

        IsoVolDescSet voldesc_set;
        voldesc_set.read(in_stream);

        voldesc_set.verify();
    }

    /**
     * Verifies that the specified string contains only valid a-characters.
     * @param [in] str The string to verify.
     * @param [in] len The string length counter in characters.
     * @throw VerificationException If the string is invalid.
     */
    void IsoVerifier::verify_a_chars(const unsigned char *str,size_t len)
    {
        for (size_t i = 0; i < len; i++)
        {
            unsigned char c = str[i];
            if (c >= ' ' && c <= '"')
                continue;
            if (c >= '%' && c <= '?')
                continue;
            if (c >= 'A' && c <= 'Z')
                continue;
            if (c == '_')
                continue;

            char str_buffer[1024];
            memcpy(str_buffer,str,len);
            str_buffer[len] = '\0';

            ckcore::tstringstream msg;
            msg << ckT("Invalid character '") << (char)c << ckT("' in the a-string \"")
                << ckcore::string::ansi_to_auto<1024>(str_buffer)
                << ckT("\".");
            throw VerificationException(msg.str(),ckT("ECMA 119: Appendix A."));
        }
    }

    /**
     * Verifies that the specified string contains only valid d-characters.
     * @param [in] str The string to verify.
     * @param [in] len The string length counter in characters.
     * @param [in] allow_sep Set to true to accept SEPARATOR 1 and SEPARATOR 2
     *                       characters.
     * @throw VerificationException If the string is invalid.
     */
    void IsoVerifier::verify_d_chars(const unsigned char *str,size_t len,bool allow_sep)
    {
        for (size_t i = 0; i < len; i++)
        {
            unsigned char c = str[i];
            if (c >= '0' && c <= '9')
                continue;
            if (c >= 'A' && c <= 'Z')
                continue;
            if (c == '_')
                continue;
            if (allow_sep)
            {
                // SEPARATOR 1 or SEPARATOR 2, in according to ECMA 119: 7.4.3.
                if (c == 0x2e || c == 0x3b)
                    continue;
            }

            char str_buffer[1024];
            memcpy(str_buffer,str,len);
            str_buffer[len] = '\0';

            ckcore::tstringstream msg;
            msg << ckT("Invalid character '") << (char)c << ckT("' in the d-string \"")
                << ckcore::string::ansi_to_auto<1024>(str_buffer)
                << ckT("\".");
            throw VerificationException(msg.str(),ckT("ECMA 119: Appendix A."));
        }
    }

    /**
     * Verifies that the specified string contains only valid Joliet
     * characters.
     * @param [in] str The string to verify.
     * @param [in] len The string length counter in characters.
     * @throw VerificationException If the string is invalid.
     */
    void IsoVerifier::verify_j_chars(const unsigned char *str,size_t len)
    {
        for (size_t i = 0; i < len; i++)
        {
            unsigned char c = str[i];
            if (c == '*' || c == '/' || c == ':' ||
                c == ';' || c == '?' || c == '\\')
            {
                char str_buffer[1024];
                memcpy(str_buffer,str,len);
                str_buffer[len] = '\0';

                ckcore::tstringstream msg;
                msg << ckT("Invalid character '") << (char)c << ckT("' in the Joliet string \"")
                    << ckcore::string::ansi_to_auto<1024>(str_buffer)
                    << ckT("\".");
                throw VerificationException(msg.str(),ckT("Joliet Specification: Allowed Character Set"));
            }
        }
    }

    /**
     * Reads a string of a-characters into a proper C-string.
     * @param [in] source The source buffer.
     * @param [in] size The size of the source buffer in bytes.
     * @param [out] target The target buffer.
     */
    void IsoVerifier::read_a_chars(const unsigned char *source,size_t size,
                                       char *target)
    {
        // Find the string length.
        size_t len = 0;
        for (int i = (int)size - 1; i >= 0; i--)
        {
            if (source[i] != 0x20)
            {
                len = i + 1;
                break;
            }
        }

        // Copy the string contents.
        memcpy(target,source,len);
        target[len] = '\0';
    }

    /**
     * Reads a string of d-characters into a proper C-string.
     * @param [in] source The source buffer.
     * @param [in] size The size of the source buffer in bytes.
     * @param [out] target The target buffer.
     */
    void IsoVerifier::read_d_chars(const unsigned char *source,size_t size,
                                       char *target)
    {
        // Find the string length.
        size_t len = 0;
        for (int i = (int)size - 1; i >= 0; i--)
        {
            if (source[i] != 0x20)
            {
                len = i + 1;
                break;
            }
        }

        // Copy the string contents.
        memcpy(target,source,len);
        target[len] = '\0';
    }

    /**
     * Reads a string of Joliet characters into a proper C-string.
     * @param [in] source The source buffer.
     * @param [in] size The size of the source buffer in bytes.
     * @param [out] target The target buffer.
     */
    void IsoVerifier::read_j_chars(const unsigned char *source,size_t size,
                                   wchar_t *target)
    {

        // FIXME: I am not sure if this length calculation is correct. I blieve
        //        the Joliet standard specifies that the padding should consist
        //        of zeros only.
        // Find the string length.
        size_t source_pos = size - 1,i,len = 0;
        for (int i = (int)(size >> 1) - 1; i > 0; i--)
        {
            wchar_t c  = source[source_pos--] << 8;
            c         |= source[source_pos--];

            if (c != 0x20)
            {
                len = i + 1;
                break;
            }
        }

        // Copy the string contents.
        for (source_pos = 0,i = 0; i < len; i++)
        {
            target[i]  = source[source_pos++] << 8;
            target[i] |= source[source_pos++];
        }

        target[len] = '\0';
    }
};
