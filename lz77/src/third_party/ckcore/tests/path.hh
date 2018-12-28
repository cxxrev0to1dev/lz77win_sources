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
#include "ckcore/types.hh"
#include "ckcore/path.hh"

class PathTestSuite : public CxxTest::TestSuite
{
public:
    void testPath()
    {
        // Full Linux paths.
        ckcore::Path path1(ckT("/this/is/a/test"));
        ckcore::Path::Iterator it = path1.begin();

        TS_ASSERT_EQUALS(*it,ckT("this"));
        ++it;
        TS_ASSERT_EQUALS(*it,ckT("is"));
        it++;
        TS_ASSERT_EQUALS(*it,ckT("a"));
        ++it;
        TS_ASSERT_EQUALS(*it,ckT("test"));

        ckcore::Path path2(ckT("/this/is/a/test/"));
        it = path2.begin();

        TS_ASSERT_EQUALS(*it,ckT("this"));
        ++it;
        TS_ASSERT_EQUALS(*it,ckT("is"));
        it++;
        TS_ASSERT_EQUALS(*it,ckT("a"));
        ++it;
        TS_ASSERT_EQUALS(*it,ckT("test"));

        // Relative path.
        ckcore::Path path3(ckT("this/is/a/test"));
        it = path3.begin();

        TS_ASSERT_EQUALS(*it,ckT("this"));
        ++it;
        TS_ASSERT_EQUALS(*it,ckT("is"));
        it++;
        TS_ASSERT_EQUALS(*it,ckT("a"));
        ++it;
        TS_ASSERT_EQUALS(*it,ckT("test"));

        // Windows paths.
        ckcore::Path path4(ckT("c:/this/is/a/test"));
        it = path4.begin();

        TS_ASSERT_EQUALS(*it,ckT("c:"));
        it++;
        TS_ASSERT_EQUALS(*it,ckT("this"));
        ++it;
        TS_ASSERT_EQUALS(*it,ckT("is"));
        it++;
        TS_ASSERT_EQUALS(*it,ckT("a"));
        ++it;
        TS_ASSERT_EQUALS(*it,ckT("test"));

#ifdef _WINDOWS
        ckcore::Path path5(ckT("c:\\this/is\\a/test"));
        it = path5.begin();

        TS_ASSERT_EQUALS(*it,ckT("c:"));
        it++;
        TS_ASSERT_EQUALS(*it,ckT("this"));
        ++it;
        TS_ASSERT_EQUALS(*it,ckT("is"));
        it++;
        TS_ASSERT_EQUALS(*it,ckT("a"));
        ++it;
        TS_ASSERT_EQUALS(*it,ckT("test"));
#else
        ckcore::Path path5(ckT("c:\\this/is\\a/test"));
        it = path5.begin();

        TS_ASSERT_EQUALS(*it,ckT("c:\\this"));
        it++;
        TS_ASSERT_EQUALS(*it,ckT("is\\a"));
        it++;
        TS_ASSERT_EQUALS(*it,ckT("test"));
#endif

        // Iterator end limit.
        it = path1.begin();
        ckcore::Path::Iterator end = path1.end();
        TS_ASSERT_EQUALS(it,it);
        it++;
        TS_ASSERT_EQUALS(it,it);
        it++;
        TS_ASSERT_EQUALS(it,it);
        it++;
        TS_ASSERT_EQUALS(it,it);
        it++;
        TS_ASSERT_EQUALS(it,end);
        ++it;
        TS_ASSERT_EQUALS(it,end);

        it = path2.begin();
        end = path2.end();
        TS_ASSERT_EQUALS(it,it);
        it++;
        TS_ASSERT_EQUALS(it,it);
        it++;
        TS_ASSERT_EQUALS(it,it);
        it++;
        TS_ASSERT_EQUALS(it,it);
        it++;
        TS_ASSERT_EQUALS(it,end);
        ++it;
        TS_ASSERT_EQUALS(it,end);
    }

    void testValid()
    {
        ckcore::Path path1(ckT("c:/this/is/a/test"));
        ckcore::Path path2(ckT("c::/this/is/a/test"));
        ckcore::Path path3(ckT("c:/this?/is/a/test"));
        ckcore::Path path4(ckT("c:/this</is/a/test"));
        ckcore::Path path5(ckT("c:/this>/is/a/test"));
        ckcore::Path path6(ckT("c:/this|/is/a/test"));
        ckcore::Path path7(ckT("c:/this\"/is/a/test"));
#ifdef _WINDOWS
        TS_ASSERT(path1.valid());
        TS_ASSERT(!path2.valid());
        TS_ASSERT(!path3.valid());
        TS_ASSERT(!path4.valid());
        TS_ASSERT(!path5.valid());
        TS_ASSERT(!path6.valid());
        TS_ASSERT(!path7.valid());
#else
        TS_ASSERT(path1.valid());
        TS_ASSERT(path2.valid());
        TS_ASSERT(path3.valid());
        TS_ASSERT(path4.valid());
        TS_ASSERT(path5.valid());
        TS_ASSERT(path6.valid());
        TS_ASSERT(path7.valid());
#endif
    }

    void testRootName()
    {
        ckcore::Path path1(ckT("/this/is/"));
#ifdef _WINDOWS
        TS_ASSERT_EQUALS(path1.root_name(),ckT(""));

        path1 = ckT("/this/is");
        TS_ASSERT_EQUALS(path1.root_name(),ckT(""));

        path1 = ckT("/this");
        TS_ASSERT_EQUALS(path1.root_name(),ckT(""));

        path1 = ckT("/");
        TS_ASSERT_EQUALS(path1.root_name(),ckT(""));

        path1 = ckT("this/is/");
        TS_ASSERT_EQUALS(path1.root_name(),ckT(""));

        path1 = ckT("c:/");
        TS_ASSERT_EQUALS(path1.root_name(),ckT("c:/"));

        path1 = ckT("c:/foo");
        TS_ASSERT_EQUALS(path1.root_name(),ckT("c:/"));

        path1 = ckT("c:\\");
        TS_ASSERT_EQUALS(path1.root_name(),ckT("c:\\"));
#else
        TS_ASSERT_EQUALS(path1.root_name(),ckT("/"));

        path1 = ckT("/this/is");
        TS_ASSERT_EQUALS(path1.root_name(),ckT("/"));

        path1 = ckT("/this");
        TS_ASSERT_EQUALS(path1.root_name(),ckT("/"));

        path1 = ckT("/");
        TS_ASSERT_EQUALS(path1.root_name(),ckT("/"));

        path1 = ckT("this/is/");
        TS_ASSERT_EQUALS(path1.root_name(),ckT(""));

        path1 = ckT("c:\\");
        TS_ASSERT_EQUALS(path1.root_name(),ckT(""));
#endif

        // Special cases.
        path1 = ckT("");
        TS_ASSERT_EQUALS(path1.root_name(),ckT(""));

        path1 = ckT("foo");
        TS_ASSERT_EQUALS(path1.root_name(),ckT(""));
    }

    void testDirName()
    {
        ckcore::Path path1(ckT("/this/is/a/test"));
        TS_ASSERT_EQUALS(path1.dir_name(),ckT("/this/is/a/"));

        ckcore::Path path2(ckT("/this/is/a/test/"));
        TS_ASSERT_EQUALS(path2.dir_name(),ckT("/this/is/a/"));

        ckcore::Path path3(ckT("/this"));
        TS_ASSERT_EQUALS(path3.dir_name(),ckT("/"));

        ckcore::Path path4(ckT("/this/is/a/test/"));
        TS_ASSERT_EQUALS(path4.dir_name(),ckT("/this/is/a/"));

        ckcore::Path path5(ckT("this"));
        TS_ASSERT_EQUALS(path5.dir_name(),ckT(""));

        ckcore::Path path6(ckT("c:/this"));
        TS_ASSERT_EQUALS(path6.dir_name(),ckT("c:/"));

        ckcore::Path path7(ckT("c:\\this"));
        ckcore::Path path8(ckT("c:\\this\\"));
#ifdef _WINDOWS
        TS_ASSERT_EQUALS(path7.dir_name(),ckT("c:\\"));
        TS_ASSERT_EQUALS(path8.dir_name(),ckT("c:\\"));
#else
        TS_ASSERT_EQUALS(path7.dir_name(),ckT(""));
        TS_ASSERT_EQUALS(path8.dir_name(),ckT(""));
#endif
    }

    void testBaseName()
    {
        ckcore::Path path1(ckT("c:/this/is/a/test"));
        TS_ASSERT_EQUALS(path1.base_name(),ckT("test"));

        ckcore::Path path2(ckT("c:/this/is/a/test/"));
        TS_ASSERT_EQUALS(path2.base_name(),ckT("test"));

        ckcore::Path path3(ckT("c:/test"));
        TS_ASSERT_EQUALS(path3.base_name(),ckT("test"));

        ckcore::Path path4(ckT("c:/"));
        TS_ASSERT_EQUALS(path4.base_name(),ckT("c:"));

        ckcore::Path path5(ckT("/test"));
        TS_ASSERT_EQUALS(path5.base_name(),ckT("test"));

        ckcore::Path path6(ckT("test"));
        TS_ASSERT_EQUALS(path6.base_name(),ckT("test"));

        ckcore::Path path7(ckT("test/"));
        TS_ASSERT_EQUALS(path7.base_name(),ckT("test"));

        ckcore::Path path8(ckT("c:\\this\\is\\a\\test"));
        ckcore::Path path9(ckT("c:\\this\\is\\a\\test/"));
        ckcore::Path path10(ckT("c:\\test"));
        ckcore::Path path11(ckT("c:\\"));
#ifdef _WINDOWS
        TS_ASSERT_EQUALS(path8.base_name(),ckT("test"));
        TS_ASSERT_EQUALS(path9.base_name(),ckT("test"));
        TS_ASSERT_EQUALS(path10.base_name(),ckT("test"));
        TS_ASSERT_EQUALS(path11.base_name(),ckT("c:"));
#else
        TS_ASSERT_EQUALS(path8.base_name(),ckT("c:\\this\\is\\a\\test"));
        TS_ASSERT_EQUALS(path9.base_name(),ckT("c:\\this\\is\\a\\test"));
        TS_ASSERT_EQUALS(path10.base_name(),ckT("c:\\test"));
        TS_ASSERT_EQUALS(path11.base_name(),ckT("c:\\"));
#endif
    }

    void testExtName()
    {
        ckcore::Path path1 = ckT("/this/is/a/test");
        TS_ASSERT_EQUALS(path1.ext_name(),ckT(""));

        path1 = ckT("/this/is/a/test/");
        TS_ASSERT_EQUALS(path1.ext_name(),ckT(""));

        path1 = ckT("/this/is/a/test.gz");
        TS_ASSERT_EQUALS(path1.ext_name(),ckT("gz"));

        path1 = ckT("/this/is/a/test.tar.gz");
        TS_ASSERT_EQUALS(path1.ext_name(),ckT("gz"));

        path1 = ckT("/this/is/a/.gz");
        TS_ASSERT_EQUALS(path1.ext_name(),ckT("gz"));

        path1 = ckT("/this/is/a/.");
        TS_ASSERT_EQUALS(path1.ext_name(),ckT(""));

        // Special cases.
        path1 = ckT(".");
        TS_ASSERT_EQUALS(path1.ext_name(),ckT(""));

        path1 = ckT("");
        TS_ASSERT_EQUALS(path1.ext_name(),ckT(""));
    }

    void testCompare()
    {
        ckcore::Path path1(ckT("/this/is/a/test/"));
        ckcore::Path path2(ckT("/this/is/a/test"));
        TS_ASSERT(path1 == path2);
        TS_ASSERT(!(path1 != path2));

        ckcore::Path path3(ckT("/this/is/a/test_"));
        TS_ASSERT(path1 != path3);
        TS_ASSERT(!(path1 == path3));

        ckcore::Path path4(ckT("/this/is/a/test/foo"));
        TS_ASSERT(path1 != path4);
        TS_ASSERT(!(path1 == path4));

        ckcore::Path path5(ckT("/this/is/a/test/foo/"));
        TS_ASSERT(path1 != path5);
        TS_ASSERT(!(path1 == path5));

        ckcore::Path path6(ckT("/this/is/a"));
        TS_ASSERT(path1 != path6);
        TS_ASSERT(!(path1 == path6));

        ckcore::Path path7(ckT("/this/is/a/"));
        TS_ASSERT(path1 != path7);
        TS_ASSERT(!(path1 == path7));

        // Window paths.
        ckcore::Path path8(ckT("/this/is/a\\test/foo/"));
        ckcore::Path path9(ckT("/this/is/a\\"));

#ifdef _WINDOWS
        TS_ASSERT(path8 == path5);
        TS_ASSERT(!(path8 != path5));
        TS_ASSERT(path8 == path4);
        TS_ASSERT(!(path8 != path4));

        TS_ASSERT(path9 == path6);
        TS_ASSERT(!(path9 != path6));
        TS_ASSERT(path9 == path7);
        TS_ASSERT(!(path9 != path7));
#else
        TS_ASSERT(path8 != path5);
        TS_ASSERT(!(path8 == path5));
        TS_ASSERT(path8 != path4);
        TS_ASSERT(!(path8 == path4));

        TS_ASSERT(path9 != path6);
        TS_ASSERT(!(path9 == path6));
        TS_ASSERT(path9 != path7);
        TS_ASSERT(!(path9 == path7));
#endif
    }

    void testAssign()
    {
        ckcore::Path path1(ckT("/this/is/a/test"));
        TS_ASSERT_EQUALS(path1,ckT("/this/is/a/test"));
        TS_ASSERT(path1 != ckT("/this/is/not/a/test"));

        path1 = ckT("/this/is/not/a/test");
        TS_ASSERT_EQUALS(path1,ckT("/this/is/not/a/test"));
        TS_ASSERT(path1 != ckT("/this/is/a/test"));

        ckcore::Path path2(ckT("/this/is/a/test"));
        path1 = path2;
        TS_ASSERT_EQUALS(path1,ckT("/this/is/a/test"));
        TS_ASSERT(path1 != ckT("/this/is/not/a/test"));
    }

    void testAppend()
    {
        // + operator.
        ckcore::Path path1(ckT("/this/is"));
        ckcore::Path path2 = path1 + ckT("a/test");
        TS_ASSERT_EQUALS(path2,ckT("/this/is/a/test"));

        path2 = path1 + ckT("/a/test");
        TS_ASSERT_EQUALS(path2,ckT("/this/is/a/test"));

        path2 = path1 + ckT("/a/test/");
        TS_ASSERT_EQUALS(path2,ckT("/this/is/a/test"));

        path1 = ckT("/this/is/");
        path2 = path1 + ckT("/a/test/");
        TS_ASSERT_EQUALS(path2,ckT("/this/is/a/test"));

        path2 = path1 + ckT("a/test/");
        TS_ASSERT_EQUALS(path2,ckT("/this/is/a/test"));

        // += operator.
        path1 += ckT("a/test");
        TS_ASSERT_EQUALS(path1,ckT("/this/is/a/test"));

        path1 = ckT("/this/is");
        path1 += ckT("/a/test");
        TS_ASSERT_EQUALS(path1,ckT("/this/is/a/test"));

        path1 = ckT("/this/is");
        path1 += ckT("/a/test/");
        TS_ASSERT_EQUALS(path1,ckT("/this/is/a/test"));

        path1 = ckT("/this/is/");
        path1 += ckT("/a/test/");
        TS_ASSERT_EQUALS(path1,ckT("/this/is/a/test"));

        path1 = ckT("/this/is/");
        path1 += ckT("a/test/");
        TS_ASSERT_EQUALS(path1,ckT("/this/is/a/test"));
    }

    void testEmpty()
    {
        // Test first constructor.
        ckcore::Path path1;
        TS_ASSERT_EQUALS(path1,ckT(""));
        TS_ASSERT_EQUALS(path1.begin(),path1.end());

        path1 = ckT("/this/is");
        TS_ASSERT_EQUALS(path1,ckT("/this/is"));

        path1 += ckT("a/test/");
        TS_ASSERT_EQUALS(path1,ckT("/this/is/a/test/"));

        // Test second constructor.
        ckcore::Path path2(ckT(""));
        TS_ASSERT_EQUALS(path2,ckT(""));
        TS_ASSERT_EQUALS(path2.begin(),path2.end());

        path2 = ckT("/this/is");
        TS_ASSERT_EQUALS(path2,ckT("/this/is"));

        path2 += ckT("a/test/");
        TS_ASSERT_EQUALS(path2,ckT("/this/is/a/test/"));
    }
};

