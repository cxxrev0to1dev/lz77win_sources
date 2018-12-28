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
#include "ckcore/convert.hh"
#include "ckcore/string.hh"

class ConvertTestSuite : public CxxTest::TestSuite
{
public:
    void testConvert()
    {
        ckcore::tchar convBuffer[ckcore::convert::INT_TO_STR_BUFLEN];
      
        // Test boolean values.
        {  // Scoping to prevent inadvertently testing the wrong variables
        const ckcore::tchar *str11 = ckT("0");
        ckcore::convert::b_to_str2(false, convBuffer);
        TS_ASSERT(!ckcore::string::astrcmp(str11,convBuffer));

        const ckcore::tchar *str21 = ckT("1");
        ckcore::convert::b_to_str2(true, convBuffer);
        TS_ASSERT(!ckcore::string::astrcmp(str21,convBuffer));
        }

        // Test 32-bit integer values.
        {  // Scoping to prevent inadvertently testing the wrong variables
        const ckcore::tint32 max32 = 2147483647;
        const ckcore::tint32 min32 = (-max32 - 1LL);
        
        const ckcore::tchar *str131 = ckT("2147483647");
        ckcore::convert::i32_to_str2(max32, convBuffer);
        TS_ASSERT(!ckcore::string::astrcmp(str131,convBuffer));

        const ckcore::tchar *str141 = ckT("-2147483648");
        ckcore::convert::i32_to_str2(min32, convBuffer);
        TS_ASSERT(!ckcore::string::astrcmp(str141,convBuffer));

        const ckcore::tchar *str151 = ckT("4294967295");
        ckcore::convert::ui32_to_str2(4294967295UL, convBuffer);
        TS_ASSERT(!ckcore::string::astrcmp(str151,convBuffer));
        }
        
        // Test 64-bit integer values.
        {  // Scoping to prevent inadvertently testing the wrong variables
        const ckcore::tint64 max64 = 9223372036854775807LL;
        const ckcore::tint64 min64 = (-max64 - 1LL);
        
        const ckcore::tchar *str61 = ckT("9223372036854775807");
        ckcore::convert::i64_to_str2(max64, convBuffer);
        TS_ASSERT(!ckcore::string::astrcmp(str61,convBuffer));

        const ckcore::tchar *str71 = ckT("-9223372036854775808");
        ckcore::convert::i64_to_str2(min64, convBuffer);
        TS_ASSERT(!ckcore::string::astrcmp(str71,convBuffer));
        

        const ckcore::tchar *str81 = ckT("18446744073709551615");
        ckcore::convert::ui64_to_str2(18446744073709551615ULL, convBuffer);
        TS_ASSERT(!ckcore::string::astrcmp(str81,convBuffer));
        }

        // sprintf function.
        const ckcore::tchar *str91 = ckT("Test: 42.");
        ckcore::tchar buffer[64];
        ckcore::convert::sprintf(buffer,sizeof(buffer),ckT("Test: %u."),42);
        TS_ASSERT(!ckcore::string::astrcmp(str91,buffer));
    }
};
