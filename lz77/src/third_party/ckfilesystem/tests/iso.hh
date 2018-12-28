/*
 * The ckFileSystem library provides core software functionality.
 * Copyright (C) 2006-2012 Christian Kindahl
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

#include <cxxtest/TestSuite.h>
#include <string.h>
#include "ckfilesystem/iso.hh"

using namespace ckfilesystem;

class IsoTestSuite : public CxxTest::TestSuite
{
public:
    void test_write_file_name_l1()
    {
        unsigned char buffer[256];

        TS_ASSERT_EQUALS(iso_write_file_name_l1(buffer, ckT(""), CHARSET_ISO), 0);

        TS_ASSERT_EQUALS(iso_write_file_name_l1(buffer, ckT("abcd"), CHARSET_ISO), 4);
        TS_ASSERT_SAME_DATA(buffer, "ABCD", 4);
        TS_ASSERT_EQUALS(iso_write_file_name_l1(buffer, ckT("abcdefghijk"), CHARSET_ISO), 8);
        TS_ASSERT_SAME_DATA(buffer, "ABCDEFGH", 8);
        TS_ASSERT_EQUALS(iso_write_file_name_l1(buffer, ckT("abcdefgh.ijk"), CHARSET_ISO), 12);
        TS_ASSERT_SAME_DATA(buffer, "ABCDEFGH.IJK", 12);
        TS_ASSERT_EQUALS(iso_write_file_name_l1(buffer, ckT("abcdefgh.ijkl"), CHARSET_ISO), 12);
        TS_ASSERT_SAME_DATA(buffer, "ABCDEFGH.IJK", 12);
        TS_ASSERT_EQUALS(iso_write_file_name_l1(buffer, ckT("abcdefghi.jkl"), CHARSET_ISO), 12);
        TS_ASSERT_SAME_DATA(buffer, "ABCDEFGH.JKL", 12);
        TS_ASSERT_EQUALS(iso_write_file_name_l1(buffer, ckT("abcdefghij.kl"), CHARSET_ISO), 11);
        TS_ASSERT_SAME_DATA(buffer, "ABCDEFGH.KL", 11);
        TS_ASSERT_EQUALS(iso_write_file_name_l1(buffer, ckT("abcdefghijk.l"), CHARSET_ISO), 10);
        TS_ASSERT_SAME_DATA(buffer, "ABCDEFGH.L", 10);
        TS_ASSERT_EQUALS(iso_write_file_name_l1(buffer, ckT("abcdefghijkl."), CHARSET_ISO), 9);
        TS_ASSERT_SAME_DATA(buffer, "ABCDEFGH.", 9);

        const char *expected_l1 =
              "______________________________________________"
            "."
              "_"
            "0123456789"
              "_______"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
              "____"
            "_"
              "_"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
              "_______________________________________________________________"
              "_______________________________________________________________"
              "______";

        for (size_t i = 1; i < 255; i++)
        {
            ckcore::tchar tmp[2] = { 0,0 };
            tmp[0] = static_cast<ckcore::tchar>(i);
            TS_ASSERT_EQUALS(iso_write_file_name_l1(buffer, tmp, CHARSET_ISO), 1);
            TS_ASSERT_EQUALS(buffer[0], static_cast<unsigned char>(expected_l1[i]));
        }
    }

    void test_write_file_name_l2()
    {
        unsigned char buffer[256];

        TS_ASSERT_EQUALS(iso_write_file_name_l2(buffer, ckT(""), CHARSET_ISO), 0);

        TS_ASSERT_EQUALS(iso_write_file_name_l2(buffer, ckT("abcd"), CHARSET_ISO), 4);
        TS_ASSERT_SAME_DATA(buffer, "ABCD", 4);
        TS_ASSERT_EQUALS(iso_write_file_name_l2(buffer, ckT("abcdefghijk"), CHARSET_ISO), 11);
        TS_ASSERT_SAME_DATA(buffer, "ABCDEFGHIJK", 11);
        TS_ASSERT_EQUALS(iso_write_file_name_l2(buffer, ckT("abcdefgh.ijk"), CHARSET_ISO), 12);
        TS_ASSERT_SAME_DATA(buffer, "ABCDEFGH.IJK", 12);
        TS_ASSERT_EQUALS(iso_write_file_name_l2(buffer, ckT("abcdefgh.ijkl"), CHARSET_ISO), 13);
        TS_ASSERT_SAME_DATA(buffer, "ABCDEFGH.IJKL", 13);
        TS_ASSERT_EQUALS(iso_write_file_name_l2(buffer, ckT("abcdefghi.jkl"), CHARSET_ISO), 13);
        TS_ASSERT_SAME_DATA(buffer, "ABCDEFGHI.JKL", 13);
        TS_ASSERT_EQUALS(iso_write_file_name_l2(buffer, ckT("abcdefghij.kl"), CHARSET_ISO), 13);
        TS_ASSERT_SAME_DATA(buffer, "ABCDEFGHIJ.KL", 13);
        TS_ASSERT_EQUALS(iso_write_file_name_l2(buffer, ckT("abcdefghijk.l"), CHARSET_ISO), 13);
        TS_ASSERT_SAME_DATA(buffer, "ABCDEFGHIJK.L", 13);
        TS_ASSERT_EQUALS(iso_write_file_name_l2(buffer, ckT("abcdefghijkl."), CHARSET_ISO), 13);
        TS_ASSERT_SAME_DATA(buffer, "ABCDEFGHIJKL.", 13);

        const char *expected_l2 =
              "______________________________________________"
            "."
              "_"
            "0123456789"
              "_______"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
              "____"
            "_"
              "_"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
              "_______________________________________________________________"
              "_______________________________________________________________"
              "______";

        for (size_t i = 1; i < 255; i++)
        {
            ckcore::tchar tmp[2] = { 0,0 };
            tmp[0] = static_cast<ckcore::tchar>(i);
            TS_ASSERT_EQUALS(iso_write_file_name_l2(buffer, tmp, CHARSET_ISO), 1);
            TS_ASSERT_EQUALS(buffer[0], static_cast<unsigned char>(expected_l2[i]));
        }
    }
};
