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
#include <algorithm>
#include <map>
#include <string.h>
#include "ckcore/filestream.hh"
#include "ckcore/linereader.hh"
#include "ckfilesystem/filesystem.hh"
#include "ckfilesystem/filetree.hh"
#include "ckfilesystem/isowriter.hh"

#ifdef TEST_SRC_DIR
#undef TEST_SRC_DIR
#endif
#define TEST_SRC_DIR        "."

using namespace ckfilesystem;

class DummyLogger : public ckcore::Log
{
public:
    void print(const ckcore::tchar *format,...) __attribute__ ((format (printf, 2, 3))) {}
    void print_line(const ckcore::tchar *format,...) __attribute__ ((format (printf, 2, 3))) {}
};

class DummyStream : public ckcore::OutStream
{
public:
    ckcore::tint64 write(const void *buffer, ckcore::tuint32 count) { return count; }
};

void read_src(const ckcore::tchar *src_path, FileSet &file_set)
{
    ckcore::FileInStream fis(src_path);
    TS_ASSERT(fis.open());

    ckcore::LineReader<char> lr(fis);
    while (!lr.end())
    {
        std::stringstream line(lr.read_line());

        std::string type, path;
        line >> type >> path;

        file_set.insert(new FileDescriptor(ckcore::string::to_auto(path).c_str(),
                                           ckT(TEST_SRC_DIR)ckT("/data/dummy"),
                                           type == "D" ? FileDescriptor::FLAG_DIRECTORY : 0));
    }
}

void read_exp(const ckcore::tchar *exp_path,
              std::vector<std::pair<ckcore::tstring, ckcore::tstring> > &exp_paths)
{
    ckcore::FileInStream fis(exp_path);
    TS_ASSERT(fis.open());

    ckcore::LineReader<char> lr(fis);
    while (!lr.end())
    {
        std::stringstream line(lr.read_line());

        std::string ext_path, int_path;
        line >> ext_path >> int_path;

        exp_paths.push_back(std::make_pair(ckcore::string::to_auto(ext_path),
                                           ckcore::string::to_auto(int_path)));
    }
}

void run_tree_test_iso(const ckcore::tchar *src_path, const ckcore::tchar *exp_path,
                       bool file_ver_info, CharacterSet char_set)
{
    DummyLogger dummy_logger;
    DummyStream dummy_stream;
    SectorOutStream dummy_sec_stream(dummy_stream);
    SectorManager dummy_sec_mgr(0);

    FileSet file_set(false);
    std::vector<std::pair<ckcore::tstring, ckcore::tstring> > exp_paths;

    read_src(src_path, file_set);
    read_exp(exp_path, exp_paths);

    FileSystem file_sys(FileSystem::TYPE_ISO, file_set);
    file_sys.set_interchange_level(Iso::LEVEL_1);
    file_sys.set_char_set(char_set);
    file_sys.set_include_file_ver_info(file_ver_info);

    FileTree file_tree(dummy_logger);
    TS_ASSERT(file_tree.create_from_file_set(file_set));
    destroy_file_set(file_set);

    IsoWriter iso_writer(dummy_logger, dummy_sec_stream, dummy_sec_mgr, file_sys, false, false);
    iso_writer.calc_names(file_tree);

    std::vector<ckcore::tstring> final_org = file_tree.serialize(NAME_ORIGINAL);
    std::vector<ckcore::tstring> final_iso = file_tree.serialize(NAME_ISO);

    TS_ASSERT_EQUALS(final_org.size(), final_iso.size());
    TS_ASSERT_EQUALS(final_org.size(), exp_paths.size());

    // Verify file and directory ordering.
    for (size_t i = 0; i < final_org.size(); i++)
    {
        TS_ASSERT_EQUALS(final_org[i], exp_paths[i].first);
    }

    // Verify internal file and directory names.
    for (size_t i = 0; i < final_org.size(); i++)
    {
        TS_ASSERT_EQUALS(final_iso[i], exp_paths[i].second);
    }
}

class FileSystemTestSuite : public CxxTest::TestSuite
{
public:
    void test_iso_file_test_01()
    {
        run_tree_test_iso(ckT(TEST_SRC_DIR)ckT("/data/iso/test-01.src"), ckT(TEST_SRC_DIR)ckT("/data/iso/test-01.exp"),
                          false, CHARSET_ISO);
    }

    void test_iso_file_test_02()
    {
        run_tree_test_iso(ckT(TEST_SRC_DIR)ckT("/data/iso/test-02.src"), ckT(TEST_SRC_DIR)ckT("/data/iso/test-02.exp"),
                          false, CHARSET_ISO);
    }

    void test_iso_file_test_03()
    {
        run_tree_test_iso(ckT(TEST_SRC_DIR)ckT("/data/iso/test-03.src"), ckT(TEST_SRC_DIR)ckT("/data/iso/test-03.exp"),
                          true, CHARSET_ISO);
    }

    void test_iso_file_test_04()
    {
        run_tree_test_iso(ckT(TEST_SRC_DIR)ckT("/data/iso/test-04.src"), ckT(TEST_SRC_DIR)ckT("/data/iso/test-04.exp"),
                          false, CHARSET_ISO);
    }

    void test_iso_file_test_05()
    {
        run_tree_test_iso(ckT(TEST_SRC_DIR)ckT("/data/iso/test-05.src"), ckT(TEST_SRC_DIR)ckT("/data/iso/test-05.exp"),
                          false, CHARSET_ISO);
    }

    void test_iso_file_test_06()
    {
        run_tree_test_iso(ckT(TEST_SRC_DIR)ckT("/data/iso/test-06.src"), ckT(TEST_SRC_DIR)ckT("/data/iso/test-06.exp"),
                          false, CHARSET_DOS);
        run_tree_test_iso(ckT(TEST_SRC_DIR)ckT("/data/iso/test-06.src"), ckT(TEST_SRC_DIR)ckT("/data/iso/test-06.exp"),
                          false, CHARSET_ASCII);
    }
};
