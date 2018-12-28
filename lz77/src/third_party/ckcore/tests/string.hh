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
#include <string.h>
#include "ckcore/types.hh"
#include "ckcore/string.hh"

class StringTestSuite : public CxxTest::TestSuite
{
public:
    void testString()
    {
        const ckcore::tchar *str1 = ckT("this is a test");
        const ckcore::tchar *str2 = ckT("this is a test");
        const ckcore::tchar *str3 = ckT("this is a longer test");
        const ckcore::tchar *str4 = ckT("THIS is A longer TEST");

        // Test comparison.
        TS_ASSERT(!ckcore::string::astrcmp(str1,str2));
        TS_ASSERT(ckcore::string::astrcmp(str1,str3));
        TS_ASSERT(!ckcore::string::astrncmp(str1,str2,14));
        TS_ASSERT(ckcore::string::astrncmp(str1,str3,14));
        TS_ASSERT(!ckcore::string::astrncmp(str1,str2,10));
        TS_ASSERT(!ckcore::string::astrncmp(str1,str3,10));
        TS_ASSERT(ckcore::string::astrncmp(str1,str3,11));
        TS_ASSERT(!ckcore::string::astrcmpi(str3,str4));
        TS_ASSERT(!ckcore::string::astrncmpi(str1,str4,10));
        TS_ASSERT(ckcore::string::astrncmpi(str1,str4,11));

        // Test length.
        TS_ASSERT_EQUALS(ckcore::string::astrlen(str1),size_t(14));
        TS_ASSERT_EQUALS(ckcore::string::astrlen(str3),size_t(21));

        // Scanning.
        const ckcore::tchar *str5 = ckT("VTS_7_53.IFO");
        ckcore::tuint32 i1 = 0,i2 = 0;
        ckcore::tchar ext[64+1];
        memset( ext, 0, sizeof(ext) );  // scanf %c does not append the null-terminator

        TS_ASSERT_EQUALS(asscanf(str5,ckT("VTS_%u_%u.%64c"),&i1,&i2,ext),3);
        TS_ASSERT_EQUALS(i1,ckcore::tuint32(7));
        TS_ASSERT_EQUALS(i2,ckcore::tuint32(53));
        TS_ASSERT(!ckcore::string::astrcmp(ext,ckT("IFO")));
    }
};
