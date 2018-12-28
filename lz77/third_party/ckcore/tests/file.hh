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
#include "ckcore/file.hh"
#include "ckcore/process.hh"

#ifdef TEST_SRC_DIR
#undef TEST_SRC_DIR
#endif
#define TEST_SRC_DIR        "."

#ifdef _WINDOWS
#define FILETESTER      ckT("bin/filetester.exe")
#else
#define FILETESTER      ckT("./bin/filetester")
#endif

class SimpleProcess: public ckcore::Process
{
public:
    void event_finished() {}
    void event_output(const std::string &block) {}
};

class FileTestSuite : public CxxTest::TestSuite
{
public:
    void testOpenClose()
    {
        ckcore::File newFilename = ckcore::File::temp(ckT("ckcore-test-file"));
      
        // Test that the file does not exist and therefore cannot be opened for reading.
        ckcore::File file1(newFilename);
        TS_ASSERT(!file1.test());

        TS_ASSERT_THROWS(file1.open2(ckcore::File::ckOPEN_READ),ckcore::Exception2);
        TS_ASSERT(!file1.test());
        TS_ASSERT(!file1.close());
        TS_ASSERT(!file1.test());

        // Create the file.
        TS_ASSERT_THROWS_NOTHING(file1.open2(ckcore::File::ckOPEN_WRITE));
        TS_ASSERT(file1.test());
        TS_ASSERT(file1.close());
        TS_ASSERT(!file1.test());

        TS_ASSERT(file1.remove());
        TS_ASSERT(!file1.test());
        TS_ASSERT(!file1.close());

        // Test opening of an existing file.
        ckcore::File file2(ckT(TEST_SRC_DIR)ckT("/data/file/0bytes"));
        TS_ASSERT(!file2.test());

        TS_ASSERT_THROWS_NOTHING(file2.open2(ckcore::File::ckOPEN_READ));
        TS_ASSERT(file2.test());
        TS_ASSERT(file2.close());
        TS_ASSERT(!file2.test());

        // We cannot open the file for writing, as under "make distcheck"
        // the directory is marked as read-only.
        //   TS_ASSERT_THROWS_NOTHING(file2.open2(ckcore::File::ckOPEN_WRITE));
        //   TS_ASSERT(file2.test());
        //   TS_ASSERT(file2.close());
        //   TS_ASSERT(!file2.test());
    }

    void testAppend()
    {
        ckcore::File newFilename1 = ckcore::File::temp( ckT("ckcore-test-file") );
        ckcore::File newFilename2 = ckcore::File::temp( ckT("ckcore-test-file") );
        TS_ASSERT( newFilename1.name() != newFilename2.name() );
        
        ckcore::File file1( newFilename1 );
        ckcore::File file2( newFilename2 );

        TS_ASSERT_THROWS_NOTHING(file1.open2(ckcore::File::ckOPEN_WRITE));
        TS_ASSERT_THROWS_NOTHING(file2.open2(ckcore::File::ckOPEN_WRITE));
        TS_ASSERT(file1.write("1234",4) != -1);
        TS_ASSERT(file2.write("1234",4) != -1);
        TS_ASSERT(file1.close());
        TS_ASSERT(file2.close());

        TS_ASSERT_THROWS_NOTHING(file1.open2(ckcore::File::ckOPEN_WRITE));
        TS_ASSERT_THROWS_NOTHING(file2.open2(ckcore::File::ckOPEN_READWRITE));
        TS_ASSERT_THROWS_NOTHING(file2.seek2(0,ckcore::File::ckFILE_END));
        TS_ASSERT(file1.write("5678",4) != -1);
        TS_ASSERT(file2.write("5678",4) != -1);
        TS_ASSERT(file1.close());
        TS_ASSERT(file2.close());

        TS_ASSERT_EQUALS(file1.size2(),4);
        TS_ASSERT_EQUALS(file2.size2(),8);

        TS_ASSERT(file1.remove());
        TS_ASSERT(file2.remove());
    }

    void testReadWrite()
    {
        ckcore::File file( ckcore::File::temp( ckT("ckcore-test-file") ) );
        TS_ASSERT_THROWS_NOTHING(file.open2(ckcore::File::ckOPEN_WRITE));
        const char out_data[] = "abcdefghijklmnopqrstuvwxyz0123456789";

        const ckcore::tint32 out_data_size = sizeof( out_data );

        ckcore::tint64 tot_write = 0;
        while (tot_write < out_data_size)
        {
            ckcore::tint64 write = file.write(out_data + tot_write, out_data_size - tot_write);
            TS_ASSERT(write != -1);

            tot_write += write;
        }

        file.close();
        TS_ASSERT_THROWS_NOTHING( file.open2(ckcore::File::ckOPEN_READ) );
        char in_data[out_data_size];

        ckcore::tint64 tot_read = 0;
        while (tot_read < out_data_size)
        {
            ckcore::tint64 read = file.read(in_data,out_data_size - tot_read);
            TS_ASSERT(read != -1);

            tot_read += read;
        }

        file.close();
        file.remove();

        TS_ASSERT_SAME_DATA(in_data,out_data,out_data_size);
    }

    void testSeekTell()
    {
        ckcore::File file(ckT(TEST_SRC_DIR)ckT("/data/file/8253bytes"));
        TS_ASSERT_THROWS_NOTHING(file.open2(ckcore::File::ckOPEN_READ));
        TS_ASSERT(file.test());

        // test exteme values.
        TS_ASSERT(file.seek2(0,ckcore::File::ckFILE_BEGIN) == 0);
        TS_ASSERT(file.tell2() == 0);
        TS_ASSERT(file.seek2(0,ckcore::File::ckFILE_CURRENT) == 0);
        TS_ASSERT(file.tell2() == 0);

        TS_ASSERT(file.seek2(0,ckcore::File::ckFILE_END) == file.size2());
        TS_ASSERT(file.tell2() == file.size2());

        int file_size = 8253;

        // test random seeking.
        for (unsigned int i = 0; i < 100; i++)
        {
            TS_ASSERT(file.seek2(0,ckcore::File::ckFILE_BEGIN) == 0);

            int seek1 = rand() % (file_size >> 1);
            int seek2 = rand() % (file_size >> 1);
            int seekckTot = seek1 + seek2;

            // seek forward from the current position.
            TS_ASSERT(file.seek2(seek1,ckcore::File::ckFILE_CURRENT) == seek1);
            TS_ASSERT(file.tell2() == seek1);

            TS_ASSERT(file.seek2(seek2,ckcore::File::ckFILE_CURRENT) == seekckTot);
            TS_ASSERT(file.tell2() == seekckTot);

            // seek backward from the current position.
            TS_ASSERT(file.seek2(-seek1,ckcore::File::ckFILE_CURRENT) == seek2);
            TS_ASSERT(file.tell2() == seek2);

            TS_ASSERT(file.seek2(-seek2,ckcore::File::ckFILE_CURRENT) == 0);
            TS_ASSERT(file.tell2() == 0);
        }
    }

    void testExistRemove()
    {
        // Create a file, then delete it.
        ckcore::File file1( ckcore::File::temp( ckT("ckcore-test-file") ) );
        TS_ASSERT(!file1.exist());
        TS_ASSERT(!file1.remove());
        TS_ASSERT_THROWS_NOTHING(file1.open2(ckcore::File::ckOPEN_WRITE));
        TS_ASSERT(file1.exist());
        TS_ASSERT(file1.remove());
        TS_ASSERT(!file1.exist());

        ckcore::File file2(ckT(TEST_SRC_DIR)ckT("/data/file/0bytes"));
        TS_ASSERT(file2.exist());

        // test static functions.
        TS_ASSERT(ckcore::File::exist(ckT(TEST_SRC_DIR)ckT("/data/file/0bytes")));
        TS_ASSERT(!ckcore::File::exist(ckT(TEST_SRC_DIR)ckT("/data/file/non-existent")));

        TS_ASSERT(!ckcore::File::remove(ckT(TEST_SRC_DIR)ckT("/data/file/non-existent")));

        // Create a file, then delete it.
        ckcore::File newFilename2 = ckcore::File::temp(ckT("ckcore-test-file"));
        ckcore::File file3( newFilename2 );
        TS_ASSERT_THROWS_NOTHING(file3.open2(ckcore::File::ckOPEN_WRITE));
        TS_ASSERT(file3.close());
        TS_ASSERT(ckcore::File::remove(newFilename2.name().c_str()));
        TS_ASSERT(!ckcore::File::remove(ckT(TEST_SRC_DIR)ckT("/data/file/non-existent")));
        TS_ASSERT(!ckcore::File::remove(ckT("")));
    }

    void testRename()
    {
        // rename file in existing folder (should succeed).
        ckcore::File newFilename1a = ckcore::File::temp( ckT("ckcore-test-file") );
        ckcore::File newFilename1b = ckcore::File::temp( ckT("ckcore-test-file") );
        ckcore::File newFilename1c = ckcore::File::temp( ckT("ckcore-test-file") );
        TS_ASSERT( newFilename1a.name() != newFilename1b.name() );
        TS_ASSERT( newFilename1a.name() != newFilename1c.name() );
      
        ckcore::File file1( newFilename1a );
        TS_ASSERT_THROWS_NOTHING(file1.open2(ckcore::File::ckOPEN_WRITE));

        TS_ASSERT(file1.rename( newFilename1b.name().c_str() ) );
        TS_ASSERT(ckcore::File::exist( newFilename1b.name().c_str() ) );
        TS_ASSERT(!file1.close());

        // rename file in new non-existing folder (should fail).
        TS_ASSERT( !ckcore::File::exist( newFilename1a.name().c_str() ) );
        ckcore::File file2( newFilename1a.name().c_str() );
        TS_ASSERT_THROWS_NOTHING(file2.open2(ckcore::File::ckOPEN_WRITE));

        ckcore::tstring newFilename1csub =newFilename1c.name();
        newFilename1csub += ckT("/non-existing-folder/new2" );

        TS_ASSERT(!file2.rename( newFilename1csub.c_str() ) );
        TS_ASSERT(!ckcore::File::exist( newFilename1csub.c_str() ) );
        TS_ASSERT(!file2.close());

        // Try to rename file to existing file (should fail).
        TS_ASSERT(!file2.rename(newFilename1b.name().c_str()));

        // Clean up.
        TS_ASSERT(file1.remove());
        TS_ASSERT(file2.remove());

        // Try to rename non-existent file.
        ckcore::File file3(ckT(TEST_SRC_DIR)ckT("/data/file/non-existent"));
        TS_ASSERT(!file3.rename(ckT(TEST_SRC_DIR)ckT("/data/file/new")));
        TS_ASSERT(!ckcore::File::exist(ckT(TEST_SRC_DIR)ckT("/data/file/new")));
        TS_ASSERT(!file3.rename(ckT(TEST_SRC_DIR)ckT("/data/file/non-existent")));
        TS_ASSERT(!ckcore::File::exist(ckT(TEST_SRC_DIR)ckT("/data/file/non-existent")));

        // Try to rename without name.
        TS_ASSERT(!file3.rename(ckT("")));

        // test static function.
        ckcore::File newFilename2a = ckcore::File::temp( ckT("ckcore-test-file") );
        ckcore::File newFilename2b = ckcore::File::temp( ckT("ckcore-test-file") );
        TS_ASSERT( newFilename2a.name() != newFilename2b.name() );
        
        ckcore::File file4( newFilename2a );
        TS_ASSERT_THROWS_NOTHING(file4.open2(ckcore::File::ckOPEN_WRITE));
        TS_ASSERT(file4.close());
        TS_ASSERT( ckcore::File::rename( newFilename2a.name().c_str(), newFilename2b.name().c_str()    ) );
        TS_ASSERT(!ckcore::File::rename( newFilename2b.name().c_str(), newFilename2b.name().c_str()    ) );
        TS_ASSERT(!ckcore::File::rename( newFilename2b.name().c_str(), ckT("")                         ) );
        TS_ASSERT(!ckcore::File::rename( newFilename2b.name().c_str(), newFilename1csub.c_str()        ) );
        
        TS_ASSERT(ckcore::File::remove ( newFilename2b.name().c_str() ) );
    }

    void testSize()
    {
        const ckcore::tchar *file_paths[] =
        {
            ckT(TEST_SRC_DIR)ckT("/data/file/0bytes"),
            ckT(TEST_SRC_DIR)ckT("/data/file/53bytes"),
            ckT(TEST_SRC_DIR)ckT("/data/file/123bytes"),
            ckT(TEST_SRC_DIR)ckT("/data/file/8253bytes")
        };

        const ckcore::tint64 file_sizes[] =
        {
            0,
            53,
            123,
            8253
        };

        for (unsigned int i = 0; i < 4; i++)
        {
            ckcore::File file(file_paths[i]);
            TS_ASSERT_EQUALS(file.size2(),file_sizes[i]);
            TS_ASSERT_THROWS_NOTHING(file.open2(ckcore::File::ckOPEN_READ));
            TS_ASSERT_EQUALS(file.size2(),file_sizes[i]);
            file.close();
            /*file.open(ckcore::File::ckOPEN_WRITE);
            TS_ASSERT_EQUALS(file.size(),file_sizes[i]);
            file.close();*/

            // test static function.
            TS_ASSERT_EQUALS(ckcore::File::size2(file_paths[i]),file_sizes[i]);
        }
    }

    void testExclusiveAccess()
    {
        // Create a new file.
        ckcore::File newFilename = ckcore::File::temp( ckT("ckcore-test-file") );
      
        ckcore::File file( newFilename );
        TS_ASSERT_THROWS_NOTHING(file.open2(ckcore::File::ckOPEN_WRITE));
        TS_ASSERT(file.close());

        TS_ASSERT_THROWS_NOTHING(file.open2(ckcore::File::ckOPEN_READ));
        TS_ASSERT(file.test());

        // Launch an external process that tries to read from the test file
        // (should succeed).
        SimpleProcess process;
        ckcore::tstring cmd_line = FILETESTER;
        cmd_line += ckT(" -r ");
        cmd_line += newFilename.name().c_str();

        TS_ASSERT(process.create(cmd_line.c_str()));
        process.wait();

        ckcore::tuint32 exit_code = -1;
        TS_ASSERT(process.exit_code(exit_code));
        TS_ASSERT_EQUALS(exit_code,ckcore::tuint32(0));

        // Launch an external process that tries to write to the test file
        // (should fail).
        cmd_line = FILETESTER;
        cmd_line += ckT(" -w ");
        cmd_line += newFilename.name().c_str();

        TS_ASSERT(process.create(cmd_line.c_str()));
        process.wait();

        exit_code = -1;
        TS_ASSERT(process.exit_code(exit_code));
        TS_ASSERT_EQUALS(exit_code,ckcore::tuint32(1));

        // Launch an external process that tries to remove the test file
        // (should fail).
        cmd_line = FILETESTER;
        cmd_line += ckT(" -d ")ckT(TEST_SRC_DIR)ckT("/data/file/new");

        TS_ASSERT(process.create(cmd_line.c_str()));
        process.wait();

        exit_code = -1;
        TS_ASSERT(process.exit_code(exit_code));
        TS_ASSERT_EQUALS(exit_code,ckcore::tuint32(1));

        // Finally, close  and remove the file.
        TS_ASSERT(file.close());
        TS_ASSERT(file.remove());
    }
};
