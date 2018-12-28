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

#include <queue>
#include <cxxtest/TestSuite.h>
#include "ckcore/types.hh"
#include "ckcore/process.hh"

#ifdef _WINDOWS
#define SMALLCLIENT     ckT("bin/smallclient.exe")
#else
#define SMALLCLIENT     ckT("./bin/smallclient")
#endif

class ProcessWrapper : public ckcore::Process
{
private:
    std::queue<std::string> block_buffer_;
    bool finished_;

public:
    ProcessWrapper() : finished_(false)
    {
    }

    ~ProcessWrapper()
    {
        invalid_inheritor_ = true;
    }

    void event_finished()
    {
        finished_ = true;
    }

    void event_output(const std::string &block)
    {
        block_buffer_.push(block);
    }

    bool finished() const
    {
        return finished_;
    }

    std::string next()
    {
        if (block_buffer_.size() == 0)
            return "";

        std::string result = block_buffer_.front();
        block_buffer_.pop();
        return result;
    }
};

class ProcessTestSuite : public CxxTest::TestSuite
{
public:
    void testFragmented()
    {
        ProcessWrapper process;

        ckcore::tstring cmd_line = SMALLCLIENT;

        TS_ASSERT(!process.finished());
        TS_ASSERT(!process.running());
        TS_ASSERT(process.create(cmd_line.c_str()));
        process.wait();
        TS_ASSERT(process.finished());
        TS_ASSERT_SAME_DATA(process.next().c_str(),"SmallClient",12);
        TS_ASSERT_SAME_DATA(process.next().c_str(),"MESSAGE 1",9);
    }

    void testInterleaved()
    {
        ProcessWrapper process;

        ckcore::tstring cmd_line = SMALLCLIENT;
        cmd_line += ckT(" -m2");

        TS_ASSERT(!process.finished());
        TS_ASSERT(!process.running());
        TS_ASSERT(process.create(cmd_line.c_str()));
        process.wait();
        TS_ASSERT(process.finished());

#ifdef _WINDOWS
        // The Windows implementation does not yet support interleaved writing.
        TS_ASSERT_SAME_DATA(process.next().c_str(),"SmallClient",12);
        TS_ASSERT_SAME_DATA(process.next().c_str(),"MESSAGE MESSAGE 1",17);
        TS_ASSERT_SAME_DATA(process.next().c_str(),"2",1);
#else
        // The Windows implementation does not yet support interleaved writing.
        TS_ASSERT_SAME_DATA(process.next().c_str(),"SmallClient",12);

        // The order of which the messages arrive may be different on different
        // operating systems. For example the first block applies for GNU/Linux
        // while the second for Mac OS X.
        std::string str1 = process.next();
        std::string str2 = process.next();

        if (str1 == "MESSAGE 1")
        {
            TS_ASSERT_SAME_DATA(str1.c_str(),"MESSAGE 1",9);
            TS_ASSERT_SAME_DATA(str2.c_str(),"MESSAGE 2",9);
        }
        else
        {
            TS_ASSERT_SAME_DATA(str2.c_str(),"MESSAGE 1",9);
            TS_ASSERT_SAME_DATA(str1.c_str(),"MESSAGE 2",9);
        }
#endif
    }

    void testBadExec()
    {
        ProcessWrapper process;

        ckcore::tstring cmd_line = ckT("ls -l");    // Does not exist in Unix as well as Windows.

        TS_ASSERT(!process.finished());
        TS_ASSERT(!process.running());
        TS_ASSERT(!process.create(cmd_line.c_str()));
        TS_ASSERT(!process.running());
        TS_ASSERT(!process.finished());
    }

    void testKill()
    {
        ProcessWrapper process;

        ckcore::tstring cmd_line = SMALLCLIENT;
        cmd_line += ckT(" -m3");    // Cause the client to sleep for 30 seconds.

        TS_ASSERT(!process.finished());
        TS_ASSERT(!process.running());
        TS_ASSERT(process.create(cmd_line.c_str()));
        TS_ASSERT(process.kill());
        TS_ASSERT(!process.finished());
    }

    void testWrite()
    {
        ProcessWrapper process;

        ckcore::tstring cmd_line = SMALLCLIENT;
        cmd_line += ckT(" -m4");    // Cause the client to read from standard input.

        TS_ASSERT(!process.finished());
        TS_ASSERT(!process.running());
        TS_ASSERT(process.create(cmd_line.c_str()));
        TS_ASSERT(!process.finished());
        TS_ASSERT(process.running());
        process.write((void *)"TEST 1\n",7);
        process.wait();
        TS_ASSERT_SAME_DATA(process.next().c_str(),"SmallClient",12);
        TS_ASSERT_SAME_DATA(process.next().c_str(),"MESSAGE 1",9);
        TS_ASSERT_SAME_DATA(process.next().c_str(),"RESPONSE 1",10);
        TS_ASSERT(process.finished());
    }

    void testDelimiters()
    {
        ProcessWrapper process;
        process.add_block_delim('G');

        ckcore::tstring cmd_line = SMALLCLIENT;

        TS_ASSERT(!process.finished());
        TS_ASSERT(!process.running());
        TS_ASSERT(process.create(cmd_line.c_str()));
        process.wait();
        TS_ASSERT(process.finished());
        TS_ASSERT_SAME_DATA(process.next().c_str(),"SmallClient",12);
        TS_ASSERT_SAME_DATA(process.next().c_str(),"MESSA",5);
        TS_ASSERT_SAME_DATA(process.next().c_str(),"E 1",3);
    }

    void testExitCode()
    {
        ProcessWrapper process;

        // Expected exit code zero.
        ckcore::tstring cmd_line = SMALLCLIENT;
        TS_ASSERT(process.create(cmd_line.c_str()));
        process.wait();
        TS_ASSERT(process.finished());

        ckcore::tuint32 exit_code = -1;
        TS_ASSERT(process.exit_code(exit_code));
        TS_ASSERT_EQUALS(exit_code,ckcore::tuint32(0));

        // Expected exit code 42.
        cmd_line += ckT(" -m5");    // Cause the client to return 42 instead of zero.

        TS_ASSERT(process.create(cmd_line.c_str()));
        process.wait();
        TS_ASSERT(process.finished());

        exit_code = -1;
        TS_ASSERT(process.exit_code(exit_code));
        TS_ASSERT_EQUALS(exit_code,ckcore::tuint32(42));

        // Expected exit code zero.
        cmd_line = SMALLCLIENT;
        TS_ASSERT(process.create(cmd_line.c_str()));
        process.wait();
        TS_ASSERT(process.finished());

        exit_code = -1;
        TS_ASSERT(process.exit_code(exit_code));
        TS_ASSERT_EQUALS(exit_code,ckcore::tuint32(0));
    }
};
