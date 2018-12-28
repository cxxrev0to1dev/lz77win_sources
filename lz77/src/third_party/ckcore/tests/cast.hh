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
#include "ckcore/cast.hh"

class CastTestSuite : public CxxTest::TestSuite
{
public:
    void testString()
    {
        const ckcore::tchar *str1 = ckT("string");
        const ckcore::tchar *str2 = ckT("-128");
        const ckcore::tchar *str3 = ckT("128");
        const ckcore::tchar *str4 = ckT("128.0");
        const ckcore::tchar *str5 = ckT("-128.0");

        ckcore::tstring res_str;
        TS_ASSERT_THROWS_NOTHING(res_str = ckcore::lexical_cast<ckcore::tstring>(str1));
        TS_ASSERT_EQUALS(res_str,ckT("string"));

        int res_int = 0;
        TS_ASSERT_THROWS_NOTHING(res_int = ckcore::lexical_cast<int>(str2));
        TS_ASSERT_EQUALS(res_int,-128);

        TS_ASSERT_THROWS_NOTHING(res_int = ckcore::lexical_cast<int>(str3));
        TS_ASSERT_EQUALS(res_int,128);

        float res_flt = 0.0f;
        TS_ASSERT_THROWS_NOTHING(res_flt = ckcore::lexical_cast<float>(str4));
        TS_ASSERT_EQUALS(res_flt,128.0f);

        TS_ASSERT_THROWS_NOTHING(res_flt = ckcore::lexical_cast<float>(str5));
        TS_ASSERT_EQUALS(res_flt,-128.0f);
    }
};
