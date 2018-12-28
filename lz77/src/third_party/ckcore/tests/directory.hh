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
#include <stdlib.h>
#include <algorithm>
#include "ckcore/types.hh"
#include "ckcore/directory.hh"

#ifdef TEST_SRC_DIR
#undef TEST_SRC_DIR
#endif
#define TEST_SRC_DIR        "."

class DirectoryTestSuite : public CxxTest::TestSuite
{
public:
    void testCreateRemove()
    {
        ckcore::tstring dirname1 = ckcore::Directory::temp().name().c_str();
        ckcore::tstring dirname1slash = dirname1;
        dirname1slash += ckT("/");
        
        ckcore::tstring dirname2 = dirname1;
        dirname2 += ckT("/new");
        ckcore::tstring dirname2slash = dirname2;
        dirname2slash += ckT("/");
      
        ckcore::Directory dir1(dirname1.c_str());
        TS_ASSERT(dir1.create());
        TS_ASSERT(dir1.remove());

        ckcore::Directory dir2(dirname1slash.c_str());
        TS_ASSERT(dir2.create());
        TS_ASSERT(dir2.remove());

        ckcore::Directory dir3(dirname2.c_str());
        TS_ASSERT(dir3.create());
        TS_ASSERT(dir3.remove());

        ckcore::Directory dir4(dirname2slash.c_str());
        TS_ASSERT(dir4.create());
        TS_ASSERT(dir4.remove());

        // This is just for clean up since the above calls to remove does not
        // remove more than one directory entry.
        ckcore::Directory dir5(dirname1.c_str());
        TS_ASSERT(dir5.remove());
    }

    void testIterator()
    {
        ckcore::Directory::Iterator it;
        ckcore::Directory dir1(ckT(TEST_SRC_DIR)ckT("/data"));
        ckcore::Directory dir2(ckT(TEST_SRC_DIR)ckT("/data/file"));

        std::list<ckcore::tstring> files1,files2;
        std::list<ckcore::tstring>::iterator it_file;

        files1.push_back(ckT("file"));

        files2.push_back(ckT("0bytes"));
        files2.push_back(ckT("53bytes"));
        files2.push_back(ckT("123bytes"));
        files2.push_back(ckT("8253bytes"));

        for (it = dir1.begin(); it != dir1.end(); it++)
        {
            it_file = std::find(files1.begin(),files1.end(),*it);
            if (it_file != files1.end())
                files1.erase(it_file);
        }

        for (it = dir2.begin(); it != dir2.end(); it++)
        {
            it_file = std::find(files2.begin(),files2.end(),*it);
            if (it_file != files2.end())
                files2.erase(it_file);
        }

        TS_ASSERT_EQUALS(files1.size(),0);
        TS_ASSERT_EQUALS(files2.size(),0);
    }
};
