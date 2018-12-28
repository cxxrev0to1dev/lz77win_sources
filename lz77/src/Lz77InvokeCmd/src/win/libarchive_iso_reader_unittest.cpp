#include <archive.h>
#include <archive_entry.h>
#include <gtest\gtest.h>
#if defined(OS_WIN_X86)
#pragma comment(lib,"gtest.lib")
#else
#pragma comment(lib,"gtest_x64.lib")
#endif

TEST(IsoExtractTest,ReadNoneIso9660) {
  struct archive_entry *ae;
  struct archive *a;
  ASSERT_TRUE((a = archive_read_new()) != NULL);
  ASSERT_TRUE(0 == archive_read_support_compression_all(a));
  ASSERT_TRUE(0 == archive_read_support_format_all(a));
  ASSERT_TRUE(0 == archive_read_open_file(a, "E:\\cn_windows_7_ultimate_with_sp1_x86_dvd_u_677486.iso", 10240));
  ASSERT_TRUE(0 == archive_read_next_header(a, &ae));
  ASSERT_EQ(true, archive_compression(a) == ARCHIVE_COMPRESSION_NONE);
  ASSERT_EQ(true, archive_format(a) == ARCHIVE_FORMAT_ISO9660);
  ASSERT_TRUE(0 == archive_read_close(a));
#if ARCHIVE_API_VERSION > 1
  ASSERT_TRUE(0 == archive_read_finish(a));
#else
  archive_read_finish(a);
#endif
}
TEST(IsoExtractTest, ReadBZip2Iso9660Rockridge) {
  struct archive_entry *ae;
  struct archive *a;
  ASSERT_TRUE((a = archive_read_new()) != NULL);
  ASSERT_TRUE(0 == archive_read_support_compression_all(a));
  ASSERT_TRUE(0 == archive_read_support_format_all(a));
  ASSERT_TRUE(0 == archive_read_open_file(a, "E:\\cn_windows_7_ultimate_with_sp1_x86_dvd_u_677486.iso", 10240));
  ASSERT_TRUE(0 == archive_read_next_header(a, &ae));
  ASSERT_EQ(true, archive_compression(a) == ARCHIVE_COMPRESSION_BZIP2);
  ASSERT_EQ(true, archive_format(a) == ARCHIVE_FORMAT_ISO9660_ROCKRIDGE);
  assert(0 == archive_read_close(a));
#if ARCHIVE_API_VERSION > 1
  ASSERT_TRUE(0 == archive_read_finish(a));
#else
  archive_read_finish(a);
#endif
}
static int copy_data(struct archive *ar, struct archive *aw){
  int r;
  const void *buff;
  size_t size;
  off_t offset;
  for (;;) {
    r = archive_read_data_block(ar, &buff, &size, &offset);
    if (r == ARCHIVE_EOF) {
      printf(archive_error_string(ar));
      return (ARCHIVE_OK);
    }
    if (r != ARCHIVE_OK)
      return (r);
    r = archive_write_data_block(aw, buff, size, offset);
    if (r != ARCHIVE_OK) {
      printf(archive_error_string(ar));
      return (r);
    }
  }
}
static void
extract(const char *filename, int do_extract, int flags)
{
  struct archive *a;
  struct archive *ext;
  struct archive_entry *entry;
  int r;

  a = archive_read_new();
  ext = archive_write_disk_new();
  archive_write_disk_set_options(ext, flags);
  /*
  * Note: archive_write_disk_set_standard_lookup() is useful
  * here, but it requires library routines that can add 500k or
  * more to a static executable.
  */
  archive_read_support_format_iso9660(a);
  /*
  * On my system, enabling other archive formats adds 20k-30k
  * each.  Enabling gzip decompression adds about 20k.
  * Enabling bzip2 is more expensive because the libbz2 library
  * isn't very well factored.
  */
  if (filename != NULL && strcmp(filename, "-") == 0)
    filename = NULL;
  if ((r = archive_read_open_file(a, filename, 10240))) {
    printf(archive_error_string(a));
    printf("\n");
    exit(r);
  }
  for (;;) {
    r = archive_read_next_header(a, &entry);
    if (r == ARCHIVE_EOF)
      break;
    if (r != ARCHIVE_OK) {
      printf(archive_error_string(a));
      printf("\n");
      exit(1);
    }
    if (1 && do_extract)
      printf("x ");
    if (1 || !do_extract)
      printf(archive_entry_pathname(entry));
    if (do_extract) {
      //const char* filename = archive_entry_pathname(entry);
      //const std::string fullOutputPath = std::string("E:\\cn_windows_7_ultimate_with_sp1_x64_dvd_u_677408\\") + filename;
      //archive_entry_set_pathname(entry, fullOutputPath.c_str());
      SetCurrentDirectoryW(L"E:\\cn_windows_7_ultimate_with_sp1_x64_dvd_u_677408");
      r = archive_write_header(ext, entry);
      if (r != ARCHIVE_OK)
        printf(archive_error_string(a));
      else
        copy_data(a, ext);
    }
    if (1 || !do_extract)
      printf("\n");
  }
  archive_read_close(a);
  archive_read_finish(a);
  exit(0);
}
TEST(IsoExtractTest, ExtractIsoFile) {
  int flags = ARCHIVE_EXTRACT_TIME;
    flags |= ARCHIVE_EXTRACT_PERM;
    flags |= ARCHIVE_EXTRACT_ACL;
    flags |= ARCHIVE_EXTRACT_FFLAGS;
  extract("E:\\cn_windows_7_ultimate_with_sp1_x64_dvd_u_677408.iso", 1, flags);
}

TEST(IsoExtractTest, ReadTest) {
	struct archive *a;
  struct archive_entry *entry;
  int r;
  a = archive_read_new();
  archive_read_support_compression_all(a);
  archive_read_support_format_all(a);
  r = archive_read_open_filename(a, "E:\\cn_windows_7_ultimate_with_sp1_x86_dvd_u_677486.iso", 10240); // Note 1
  if (r != ARCHIVE_OK)
    return;
  while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
    const char* filename = archive_entry_pathname(entry);
    archive_read_data_skip(a);
  }
  r = archive_read_close(a);
  if (r != ARCHIVE_OK)
    return;
  archive_read_finish(a);
}

int main() {
  RUN_ALL_TESTS();
  return 0;
}
