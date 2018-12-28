/*
 * The ckCore library provides core software functionality.
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
#include "ckcore/types.hh"
#include "ckcore/linereader.hh"
#include "ckcore/filestream.hh"

#ifndef TEST_SRC_DIR
#define TEST_SRC_DIR        "."
#endif

class LineReaderTestSuite : public CxxTest::TestSuite
{
public:
    void testAnsi()
    {
        // Read ANSI encoded file with no ending line break.

        ckcore::FileInStream fis(ckT(TEST_SRC_DIR)ckT("/data/linereader/text_ansi.txt"));
        TS_ASSERT(fis.open());

        ckcore::LineReader<char> lr(fis);
        TS_ASSERT_EQUALS(lr.encoding(),ckcore::LineReader<char>::ckENCODING_ANSI);
        TS_ASSERT(!lr.end());

        TS_ASSERT_SAME_DATA(lr.read_line().c_str(),"Line 1",6);
        TS_ASSERT(!lr.end());
        TS_ASSERT_SAME_DATA(lr.read_line().c_str(),"Line 2",6);
        TS_ASSERT(!lr.end());
        TS_ASSERT_SAME_DATA(lr.read_line().c_str(),"Line 3",6);
        TS_ASSERT(!lr.end());
        TS_ASSERT_SAME_DATA(lr.read_line().c_str(),"Line 4",6);
        TS_ASSERT(lr.end());

        TS_ASSERT(fis.close());
    }

    void testAnsiElb()
    {
        // Read ANSI encoded file with ending line break.
        ckcore::FileInStream fis(ckT(TEST_SRC_DIR)ckT("/data/linereader/text_ansi_elb.txt"));
        TS_ASSERT(fis.open());

        ckcore::LineReader<char> lr(fis);
        TS_ASSERT_EQUALS(lr.encoding(),ckcore::LineReader<char>::ckENCODING_ANSI);
        TS_ASSERT(!lr.end());

        TS_ASSERT_SAME_DATA(lr.read_line().c_str(),"Line 1",6);
        TS_ASSERT(!lr.end());
        TS_ASSERT_SAME_DATA(lr.read_line().c_str(),"Line 2",6);
        TS_ASSERT(!lr.end());
        TS_ASSERT_SAME_DATA(lr.read_line().c_str(),"Line 3",6);
        TS_ASSERT(!lr.end());
        TS_ASSERT_SAME_DATA(lr.read_line().c_str(),"Line 4",6);
        TS_ASSERT(lr.end());

        TS_ASSERT(fis.close());
    }

    void testUtf8()
    {
        // Read UTF-8 encoded file with no ending line break.
        ckcore::FileInStream fis(ckT(TEST_SRC_DIR)ckT("/data/linereader/text_utf8.txt"));
        TS_ASSERT(fis.open());

        ckcore::LineReader<char> lr(fis);
        TS_ASSERT_EQUALS(lr.encoding(),ckcore::LineReader<char>::ckENCODING_UTF8);
        TS_ASSERT(!lr.end());

        TS_ASSERT_SAME_DATA(lr.read_line().c_str(),"Line 1",6);
        TS_ASSERT(!lr.end());
        TS_ASSERT_SAME_DATA(lr.read_line().c_str(),"Line 2",6);
        TS_ASSERT(!lr.end());
        TS_ASSERT_SAME_DATA(lr.read_line().c_str(),"Line 3",6);
        TS_ASSERT(!lr.end());
        TS_ASSERT_SAME_DATA(lr.read_line().c_str(),"Line 4",6);
        TS_ASSERT(lr.end());

        TS_ASSERT(fis.close());
    }

    void testUtf8Elb()
    {
        // Read UTF-8 encoded file with ending line break.
        ckcore::FileInStream fis(ckT(TEST_SRC_DIR)ckT("/data/linereader/text_utf8_elb.txt"));
        TS_ASSERT(fis.open());

        ckcore::LineReader<char> lr(fis);
        TS_ASSERT_EQUALS(lr.encoding(),ckcore::LineReader<char>::ckENCODING_UTF8);
        TS_ASSERT(!lr.end());

        TS_ASSERT_SAME_DATA(lr.read_line().c_str(),"Line 1",6);
        TS_ASSERT(!lr.end());
        TS_ASSERT_SAME_DATA(lr.read_line().c_str(),"Line 2",6);
        TS_ASSERT(!lr.end());
        TS_ASSERT_SAME_DATA(lr.read_line().c_str(),"Line 3",6);
        TS_ASSERT(!lr.end());
        TS_ASSERT_SAME_DATA(lr.read_line().c_str(),"Line 4",6);
        TS_ASSERT(lr.end());

        TS_ASSERT(fis.close());
    }

    void testUtfLe16()
    {
        // Read UTF-16 encoded file with no ending line break.
        ckcore::FileInStream fis(ckT(TEST_SRC_DIR)ckT("/data/linereader/text_utf16le.txt"));
        TS_ASSERT(fis.open());

        ckcore::LineReader<short> lr(fis);
        TS_ASSERT_EQUALS(lr.encoding(),ckcore::LineReader<short>::ckENCODING_UTF16LE);
        TS_ASSERT(!lr.end());

        TS_ASSERT_SAME_DATA(lr.read_line().c_str(),"L\0i\0n\0e\0 \0\x31\0",12);
        TS_ASSERT(!lr.end());
        TS_ASSERT_SAME_DATA(lr.read_line().c_str(),"L\0i\0n\0e\0 \0\x32\0",12);
        TS_ASSERT(!lr.end());
        TS_ASSERT_SAME_DATA(lr.read_line().c_str(),"L\0i\0n\0e\0 \0\x33\0",12);
        TS_ASSERT(!lr.end());
        TS_ASSERT_SAME_DATA(lr.read_line().c_str(),"L\0i\0n\0e\0 \0\x34\0",12);
        TS_ASSERT(lr.end());

        TS_ASSERT(fis.close());
    }

    void testUtf16LeElb()
    {
        // Read UTF-16 encoded file with ending line break.
        ckcore::FileInStream fis(ckT(TEST_SRC_DIR)ckT("/data/linereader/text_utf16le_elb.txt"));
        TS_ASSERT(fis.open());

        ckcore::LineReader<short> lr(fis);
        TS_ASSERT_EQUALS(lr.encoding(),ckcore::LineReader<short>::ckENCODING_UTF16LE);
        TS_ASSERT(!lr.end());

        TS_ASSERT_SAME_DATA(lr.read_line().c_str(),"L\0i\0n\0e\0 \0\x31\0",12);
        TS_ASSERT(!lr.end());
        TS_ASSERT_SAME_DATA(lr.read_line().c_str(),"L\0i\0n\0e\0 \0\x32\0",12);
        TS_ASSERT(!lr.end());
        TS_ASSERT_SAME_DATA(lr.read_line().c_str(),"L\0i\0n\0e\0 \0\x33\0",12);
        TS_ASSERT(!lr.end());
        TS_ASSERT_SAME_DATA(lr.read_line().c_str(),"L\0i\0n\0e\0 \0\x34\0",12);
        TS_ASSERT(lr.end());

        TS_ASSERT(fis.close());
    }
};
