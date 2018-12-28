// Lz77ConvFile.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <assert.h>
#include <string>
#include <fstream>
#include <Windows.h>
#include "compressor/lib7zip_compressor.h"
#include <gtest\gtest.h>
#if defined(OS_WIN_X86)
#pragma comment(lib,"gtest.lib")
#else
#pragma comment(lib,"gtest_x64.lib")
#endif

extern "C" IMAGE_DOS_HEADER __ImageBase;

void GetDir(std::wstring& dir_path) {
  wchar_t dll_folder[MAX_PATH + 1] = { 0 };
  GetModuleFileNameW((HMODULE)&__ImageBase, dll_folder, MAX_PATH);
  std::wstring dll_dirs(dll_folder);
  std::wstring::size_type pos = std::wstring(dll_dirs).find_last_of(L"\\/");
  dll_dirs = std::wstring(dll_dirs).substr(0, pos);
  dir_path = dll_dirs;
}

void CreateZipArc(std::vector<std::wstring> file_list,
  const std::wstring& out_arc,
  const std::wstring& pwd){
  compressor::ArchiveCompressor archive_compressor(nullptr);
  EXPECT_FALSE(archive_compressor.IsSupportedARCExt(L"zip"));
  archive_compressor.compressor(file_list, out_arc, pwd);
  EXPECT_TRUE(archive_compressor.IsCompressOK());
}
TEST(CreateComponentsArcToCppSource, LZ77ComponentsZipPackage) {
  wchar_t* components_list[] = {
    { L"\\res" },
#if defined(OS_WIN_X86)
    { L"\\base.dll" },
    { L"\\compressor.dll" },
    { L"\\crypt.dll" },
    { L"\\CryptApp.exe" },
    { L"\\libcrypto-1_1.dll" },
    { L"\\libmikmod.dll" },
    { L"\\libssl-1_1.dll" },
    { L"\\Lz77Ext.dll" },
    { L"\\Lz77InvokeCmd.exe" },
    { L"\\Lz77Lang.dll" },
    { L"\\Lz77RegTestCOM.exe" },
#else
    { L"\\base_x64.dll" },
    { L"\\compressor_x64.dll" },
    { L"\\crypt_x64.dll" },
    { L"\\CryptApp_x64.exe" },
    { L"\\libcrypto-1_1-x64.dll" },
    { L"\\libmikmod_x64.dll" },
    { L"\\libssl-1_1-x64.dll" },
    { L"\\Lz77Ext_x64.dll" },
    { L"\\Lz77InvokeCmd_x64.exe" },
    { L"\\Lz77Lang_x64.dll" },
    { L"\\Lz77RegTestCOM_x64.exe" },
#endif
    nullptr
  };
  std::wstring dir;
  GetDir(dir);
  std::vector<std::wstring> file_list;
  for (size_t i = 0; components_list[i]!=nullptr; i++){
    std::wstring temp = dir + components_list[i];
    file_list.push_back(temp);
  }
#if defined(OS_WIN_X86)
  std::wstring out_arc = dir + L"\\installer.zip";
#else
  std::wstring out_arc = dir + L"\\installer_x64.zip";
#endif
  DeleteFileW(out_arc.c_str());
  CreateZipArc(file_list, out_arc, L"");
}

TEST(CreateComponentsArcToCppSource, ZipPackageToCppSources) {
  std::wstring out;
  GetDir(out);
#if defined(OS_WIN_X86)
  std::wstring in_arc = out + L"\\installer.zip";
  std::string c_source = "unsigned char hex_c_x86[] = {\n";
#else
  std::wstring in_arc = out + L"\\installer_x64.zip";
  std::string c_source = "static unsigned char hex_c_x64[] = {\n";
#endif
  FILE* f = _wfopen(in_arc.c_str(), L"rb");
  ASSERT_NE(nullptr, f);
  unsigned long n = 0;
  while (!feof(f)) {
    unsigned char c;
    if (fread(&c, 1, 1, f) == 0)
      break;
    char buffer[128] = { 0 };
    sprintf(buffer, "0x%.2X,", (int)c);
    c_source.append(buffer);
    ++n;
    if (n % 10 == 0) c_source.append("\n");
  }
  fclose(f);
  c_source.append("};\n");
  char buffer[128] = { 0 };
#if defined(OS_WIN_X86)
  sprintf(buffer, "static unsigned long hex_c_x86_len = 0x%x;\n", n);
  std::wstring out_c_headers = out + L"\\installer.h";
#else
  sprintf(buffer, "static unsigned long hex_c_x64_len = 0x%x;\n", n);
  std::wstring out_c_headers = out + L"\\installer_x64.h";
#endif
  c_source.append(buffer);
  std::ofstream out_stream(out_c_headers, std::ios::binary | std::ios::out);
  ASSERT_EQ(true, out_stream.is_open());
  out_stream.write(c_source.c_str(), c_source.length());
  out_stream.close();
  out_stream.clear();
}

int main(int argc,char** argv){
  testing::InitGoogleTest(&argc, argv);
  RUN_ALL_TESTS();
  getchar();
  return 0;
}

